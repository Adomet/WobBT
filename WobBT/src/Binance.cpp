#include "Binance.h"
#include "Debug.h"
#include "Strategy.h"

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <winhttp.h>
#include <wincrypt.h>
#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "advapi32.lib")
#endif

#include <sstream>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <algorithm>
#include <iomanip>
#include <ctime>
#include <mutex>

#ifndef _WIN32
#include <curl/curl.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#endif


BinanceBroker::BinanceBroker(const BinanceConfig& config)
    : m_config(config)
    , m_ohlc(std::vector<double>{}, std::vector<double>{}, std::vector<double>{},
              std::vector<double>{}, std::vector<double>{}, config.candleType)
{
}

BinanceBroker::~BinanceBroker()
{
    m_running = false;
}

void BinanceBroker::appendCandle(double o, double h, double l, double c, double v)
{
    m_ohlc.open.push_back(o);
    m_ohlc.high.push_back(h);
    m_ohlc.low.push_back(l);
    m_ohlc.close.push_back(c);
    m_ohlc.volume.push_back(v);
}

void BinanceBroker::removeLastCandle()
{
    if (m_ohlc.close.empty())
        return;
    m_ohlc.open.pop_back();
    m_ohlc.high.pop_back();
    m_ohlc.low.pop_back();
    m_ohlc.close.pop_back();
    m_ohlc.volume.pop_back();
}

void BinanceBroker::fetchInitialKlines()
{
    m_ohlc.open.clear();
    m_ohlc.high.clear();
    m_ohlc.low.clear();
    m_ohlc.close.clear();
    m_ohlc.volume.clear();

    std::string interval = OHLC::candleString(m_config.candleType);
    std::string url = "https://api.binance.com/api/v3/klines?symbol=" + m_config.symbol
        + "&interval=" + interval + "&limit=" + std::to_string(m_config.warmupCandles);

    Debug::Log("Fetching initial klines: " + url);
    std::string resp = httpGet(url);
    if (resp.empty())
    {
        Debug::Log("Failed to fetch initial klines");
        return;
    }

    size_t off = 0;
    const char* buf = resp.c_str();
    size_t len = resp.size();
    long long prevTime = 0, lastTime = 0;

    while (off < len)
    {
        double o = 0, h = 0, l = 0, c = 0, v = 0;
        prevTime = lastTime;
        if (!parseKline(buf, len, off, o, h, l, c, v, &lastTime))
            break;
        appendCandle(o, h, l, c, v);
    }
    // Drop last candle: it's still forming (incomplete).
    // m_lastKlineTime = openTime of the last KEPT candle, not the popped one.
    if (m_ohlc.close.size() >= 2)
    {
        removeLastCandle();
        m_lastKlineTime = prevTime;
    }

    Debug::Log("Loaded " + std::to_string(m_ohlc.close.size()) + " warmup candles");
}

bool BinanceBroker::parseKline(const char* buf, size_t len, size_t& off, double& o, double& h, double& l, double& c, double& v, long long* outOpenTime)
{
    while (off < len && buf[off] != '[') off++;
    if (off >= len) return false;

    auto parseNum = [&]() -> double {
        while (off < len && (buf[off] == '[' || buf[off] == ',' || buf[off] == ' ' || buf[off] == '"')) off++;
        if (off >= len) return 0;
        char* end = nullptr;
        double val = std::strtod(buf + off, &end);
        if (end) off = (size_t)(end - buf);
        return val;
    };

    off++;
    long long openTime = (long long)parseNum();
    if (outOpenTime) *outOpenTime = openTime;
    o = parseNum();
    h = parseNum();
    l = parseNum();
    c = parseNum();
    v = parseNum();

    // Skip to next '[' or end of array
    while (off < len && buf[off] != '[' && buf[off] != ']') off++;

    return (o != 0 || c != 0);
}

bool BinanceBroker::fetchLatestKlines()
{
    std::string interval = OHLC::candleString(m_config.candleType);
    std::string url = "https://api.binance.com/api/v3/klines?symbol=" + m_config.symbol        + "&interval=" + interval + "&limit=2";
    std::string resp = httpGet(url);
    if (resp.empty()) return false;

    const char* buf = resp.c_str();
    size_t len = resp.size();
    double o = 0, h = 0, l = 0, c = 0, v = 0;
    long long klineTime = 0;

    // limit=2 returns [closed, forming] — parse only the first (closed) candle
    size_t off = 0;
    if (!parseKline(buf, len, off, o, h, l, c, v, &klineTime))
        return false;

    if (klineTime > m_lastKlineTime && (o != 0 || c != 0))
    {
        m_lastKlineTime = klineTime;
        m_lastProcessedOpenTime = klineTime;
        m_lastProcessedClose = c;
        appendCandle(o, h, l, c, v);
        return true;
    }
    return false;
}

void BinanceBroker::refreshState()
{
    fetchWalletBalances(m_cachedState.coin, m_cachedState.cash);
}

BrokerState BinanceBroker::getState()
{
    return m_cachedState;
}

void BinanceBroker::logLiveCandles(const BrokerState& state) const
{
    std::ostringstream log;
    log << std::fixed << std::setprecision(2);
    log << formatCandleTime(m_lastProcessedOpenTime) << " - " << m_config.symbol
        << " | Coin " << state.coin
        << " | Cash " << state.cash
        << " | C: " << m_lastProcessedClose;
    Debug::Log(log.str());
}

void BinanceBroker::executeOrder(bool isBuy, double price)
{
    if (m_config.apiKey.empty() || m_config.apiSecret.empty())
    {
        Debug::Log("Binance API keys not set - order NOT executed (simulation only)");
        return;
    }

    if (isBuy)
        m_cachedState.lastBuyPrice = price;
    else
        m_cachedState.lastBuyPrice = -1;

    std::string side = isBuy ? "BUY" : "SELL";
    std::string type = "MARKET";

    double buyCash = m_cachedState.cash * 0.999;
    double SellCoin = floor(m_cachedState.coin / 0.01) * 0.01;

    std::ostringstream qs;
    qs << std::fixed << std::setprecision(2);
    qs << "symbol=" << m_config.symbol << "&side=" << side << "&type=" << type;
    if (isBuy)
        qs << "&quoteOrderQty=" << buyCash;
    else
        qs << "&quantity=" << SellCoin;

    qs << "&timestamp=" << (std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());

    std::string queryString = qs.str();
    std::string signature = signRequest(queryString);
    queryString += "&signature=" + signature;

    std::string url = "https://api.binance.com/api/v3/order";
    std::string resp = httpPost(url, queryString);

    if (!resp.empty())
        Debug::Log("Order " + side + " @ " + std::to_string(price) + " -> " + resp.substr(0, 200));
    else
        Debug::Log("Order " + side + " request failed");
}

void BinanceBroker::runLive(Cerebro& cerebro)
{
    fetchInitialKlines();
    if (m_ohlc.close.size() < 2)
    {
        Debug::Log("Not enough initial data for live trading");
        return;
    }

    Strategy* strat = cerebro.getStrategy();
    if (!strat)
    {
        Debug::Log("Cerebro has no strategy");
        return;
    }

    strat->m_Data = &m_ohlc;
    strat->m_warmupMode = false;
    strat->m_broker = this;
    strat->init();

    const int pollIntervalSec = std::max(1, m_config.pollIntervalSec);
    Debug::Log("Live trading started. Polling every " + std::to_string(pollIntervalSec) + "s");
    refreshState();
    logLiveCandles(getState());

    while (m_running)
    {
        if (fetchLatestKlines())
        {
            size_t lastIdx = m_ohlc.close.size() - 1;
            if (lastIdx >= 1)
            {
                refreshState();
                strat->deleteElements();
                strat->init();
                strat->m_isOrdered = false;
                strat->next((int)lastIdx);
                logLiveCandles(getState());
            }
        }

        for (int i = 0; i < pollIntervalSec && m_running; i++)
            std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

#ifdef _WIN32
std::string BinanceBroker::httpGet(const std::string& url) const
{
    return httpGet(url, "");
}

std::string BinanceBroker::httpGet(const std::string& url, const std::string& extraHeaders) const
{
    URL_COMPONENTS uc = {};
    wchar_t host[256] = {}, path[1024] = {};
    uc.dwStructSize = sizeof(uc);
    uc.lpszHostName = host;
    uc.dwHostNameLength = 256;
    uc.lpszUrlPath = path;
    uc.dwUrlPathLength = 1024;

    std::wstring wurl(url.begin(), url.end());
    if (!WinHttpCrackUrl(wurl.c_str(), (DWORD)wurl.size(), 0, &uc))
        return "";

    HINTERNET hSession = WinHttpOpen(L"WobBT/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, 0, 0, 0);
    if (!hSession) return "";

    HINTERNET hConnect = WinHttpConnect(hSession, host, uc.nPort ? uc.nPort : 443, 0);
    if (!hConnect) { WinHttpCloseHandle(hSession); return ""; }

    DWORD flags = (uc.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path, 0, 0, 0, flags);
    if (!hRequest) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return ""; }

    std::wstring wheaders;
    if (!extraHeaders.empty())
        wheaders.assign(extraHeaders.begin(), extraHeaders.end());

    LPCWSTR headers = extraHeaders.empty() ? WINHTTP_NO_ADDITIONAL_HEADERS : wheaders.c_str();
    DWORD headerLen = extraHeaders.empty() ? 0 : (DWORD)-1L;
    if (!WinHttpSendRequest(hRequest, headers, headerLen, 0, 0, 0, 0))
    {
        WinHttpCloseHandle(hRequest); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession);
        return "";
    }

    if (!WinHttpReceiveResponse(hRequest, 0))
    {
        WinHttpCloseHandle(hRequest); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession);
        return "";
    }

    std::string result;
    DWORD available = 0;
    do
    {
        if (!WinHttpQueryDataAvailable(hRequest, &available) || available == 0) break;
        std::vector<char> buf(available);
        DWORD read = 0;
        if (!WinHttpReadData(hRequest, buf.data(), available, &read)) break;
        result.append(buf.data(), read);
    } while (available > 0);

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return result;
}

bool BinanceBroker::fetchWalletBalances(double& coin, double& cash) const
{
    if (m_config.apiKey.empty() || m_config.apiSecret.empty())
    {
        Debug::Log("fetchWalletBalances: missing API key/secret");
        return false;
    }

    const auto assets = splitSymbolAssets();
    if (assets.first.empty() || assets.second.empty())
    {
        Debug::Log("fetchWalletBalances: could not split symbol assets from " + m_config.symbol);
        return false;
    }

    const long long ts = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    std::string queryString = "timestamp=" + std::to_string(ts);
    std::string signature = signRequest(queryString);
    if (signature.empty())
    {
        Debug::Log("fetchWalletBalances: request signature generation failed");
        return false;
    }

    std::string url = "https://api.binance.com/api/v3/account?" + queryString + "&signature=" + signature;
    std::string headers = "X-MBX-APIKEY: " + m_config.apiKey + "\r\n";
    std::string resp = httpGet(url, headers);
    if (resp.empty())
    {
        Debug::Log("fetchWalletBalances: /api/v3/account returned empty response");
        return false;
    }
    if (resp.find("\"code\"") != std::string::npos || resp.find("\"msg\"") != std::string::npos)
    {
        Debug::Log("fetchWalletBalances: Binance error payload -> " + resp.substr(0, 300));
    }

    auto extractFree = [&](const std::string& asset) -> double {
        std::string marker = "\"asset\":\"" + asset + "\"";
        size_t pos = resp.find(marker);
        if (pos == std::string::npos)
            return -1.0;
        size_t freePos = resp.find("\"free\":\"", pos);
        if (freePos == std::string::npos)
            return -1.0;
        freePos += 8; // len("\"free\":\"")
        size_t end = resp.find("\"", freePos);
        if (end == std::string::npos || end <= freePos)
            return -1.0;
        return std::strtod(resp.substr(freePos, end - freePos).c_str(), nullptr);
    };

    const double coinVal = extractFree(assets.first);
    const double cashVal = extractFree(assets.second);
    if (coinVal < 0.0 || cashVal < 0.0)
    {
        Debug::Log("fetchWalletBalances: failed to parse balances for assets " + assets.first + "/" + assets.second);
        return false;
    }

    coin = coinVal;
    cash = cashVal;
    Debug::Log("fetchWalletBalances: success | " + assets.first + "=" + std::to_string(coin)
        + " | " + assets.second + "=" + std::to_string(cash));
    return true;
}

long long BinanceBroker::candleIntervalMs() const
{
    switch (m_config.candleType)
    {
    case OHLC::m1:  return 60LL * 1000LL;
    case OHLC::m5:  return 5LL * 60LL * 1000LL;
    case OHLC::m15: return 15LL * 60LL * 1000LL;
    case OHLC::m30: return 30LL * 60LL * 1000LL;
    case OHLC::h1:  return 60LL * 60LL * 1000LL;
    case OHLC::h2:  return 2LL * 60LL * 60LL * 1000LL;
    case OHLC::h4:  return 4LL * 60LL * 60LL * 1000LL;
    case OHLC::h12: return 12LL * 60LL * 60LL * 1000LL;
    case OHLC::d1:  return 24LL * 60LL * 60LL * 1000LL;
    case OHLC::w1:  return 7LL * 24LL * 60LL * 60LL * 1000LL;
    default:        return 15LL * 60LL * 1000LL;
    }
}

std::string BinanceBroker::formatCandleTime(long long openTimeMs) const
{
    const long long closeTimeMs = openTimeMs + candleIntervalMs();
    std::time_t tt = (std::time_t)(closeTimeMs / 1000LL);
    std::tm tmVal{};
#ifdef _WIN32
    localtime_s(&tmVal, &tt);
#else
    localtime_r(&tt, &tmVal);
#endif
    std::ostringstream ss;
    ss << std::put_time(&tmVal, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::pair<std::string, std::string> BinanceBroker::splitSymbolAssets() const
{
    static const char* knownQuotes[] = { "USDT", "BUSD", "USDC", "BTC", "ETH", "BNB", "TRY", "EUR" };
    for (const char* quote : knownQuotes)
    {
        const std::string q(quote);
        if (m_config.symbol.size() > q.size() &&
            m_config.symbol.compare(m_config.symbol.size() - q.size(), q.size(), q) == 0)
        {
            return { m_config.symbol.substr(0, m_config.symbol.size() - q.size()), q };
        }
    }
    return { "", "" };
}

std::string BinanceBroker::httpPost(const std::string& url, const std::string& queryString)
{
    URL_COMPONENTS uc = {};
    wchar_t host[256] = {}, path[1024] = {};
    uc.dwStructSize = sizeof(uc);
    uc.lpszHostName = host;
    uc.dwHostNameLength = 256;
    uc.lpszUrlPath = path;
    uc.dwUrlPathLength = 1024;

    std::wstring wurl(url.begin(), url.end());
    if (!WinHttpCrackUrl(wurl.c_str(), (DWORD)wurl.size(), 0, &uc))
        return "";

    HINTERNET hSession = WinHttpOpen(L"WobBT/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, 0, 0, 0);
    if (!hSession) return "";

    HINTERNET hConnect = WinHttpConnect(hSession, host, uc.nPort ? uc.nPort : 443, 0);
    if (!hConnect) { WinHttpCloseHandle(hSession); return ""; }

    DWORD flags = (uc.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", path, 0, 0, 0, flags);
    if (!hRequest) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return ""; }

    std::wstring headers = L"Content-Type: application/x-www-form-urlencoded\r\n"
        L"X-MBX-APIKEY: ";
    std::string key = m_config.apiKey;
    headers += std::wstring(key.begin(), key.end());
    headers += L"\r\n";

    if (!WinHttpSendRequest(hRequest, headers.c_str(), (DWORD)-1L,
        (LPVOID)queryString.c_str(), (DWORD)queryString.size(), (DWORD)queryString.size(), 0))
    {
        WinHttpCloseHandle(hRequest); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession);
        return "";
    }

    if (!WinHttpReceiveResponse(hRequest, 0))
    {
        WinHttpCloseHandle(hRequest); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession);
        return "";
    }

    std::string result;
    DWORD available = 0;
    do
    {
        if (!WinHttpQueryDataAvailable(hRequest, &available) || available == 0) break;
        std::vector<char> buf(available);
        DWORD read = 0;
        if (!WinHttpReadData(hRequest, buf.data(), available, &read)) break;
        result.append(buf.data(), read);
    } while (available > 0);

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return result;
}

std::string BinanceBroker::signRequest(const std::string& queryString) const
{
    const size_t blockSize = 64;
    std::string key = m_config.apiSecret;
    if (key.size() > blockSize)
    {
        HCRYPTPROV hp = 0;
        HCRYPTHASH hh = 0;
        if (!CryptAcquireContext(&hp, 0, 0, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) return "";
        if (!CryptCreateHash(hp, CALG_SHA_256, 0, 0, &hh)) { CryptReleaseContext(hp, 0); return ""; }
        CryptHashData(hh, (const BYTE*)key.c_str(), (DWORD)key.size(), 0);
        BYTE buf[32]; DWORD blen = 32;
        if (!CryptGetHashParam(hh, HP_HASHVAL, buf, &blen, 0))
        {
            CryptDestroyHash(hh); CryptReleaseContext(hp, 0); return "";
        }
        CryptDestroyHash(hh); CryptReleaseContext(hp, 0);
        key.assign((char*)buf, 32);
    }
    if (key.size() < blockSize)
        key.append(blockSize - key.size(), 0);

    std::string ipad(blockSize, 0x36);
    std::string opad(blockSize, 0x5c);

    for (size_t i = 0; i < blockSize; i++)
    {
        ipad[i] ^= (uint8_t)key[i];
        opad[i] ^= (uint8_t)key[i];
    }

    // SHA256(ipad || queryString)
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHashInner = 0;
    if (!CryptAcquireContext(&hProv, 0, 0, PROV_RSA_AES, CRYPT_VERIFYCONTEXT))
        return "";
    if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHashInner))
    {
        CryptReleaseContext(hProv, 0);
        return "";
    }
    CryptHashData(hHashInner, (const BYTE*)ipad.c_str(), (DWORD)ipad.size(), 0);
    CryptHashData(hHashInner, (const BYTE*)queryString.c_str(), (DWORD)queryString.size(), 0);

    BYTE innerHash[32];
    DWORD innerLen = 32;
    if (!CryptGetHashParam(hHashInner, HP_HASHVAL, innerHash, &innerLen, 0))
    {
        CryptDestroyHash(hHashInner);
        CryptReleaseContext(hProv, 0);
        return "";
    }
    CryptDestroyHash(hHashInner);

    // SHA256(opad || innerHash)
    HCRYPTHASH hHashOuter = 0;
    if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHashOuter))
    {
        CryptReleaseContext(hProv, 0);
        return "";
    }
    CryptHashData(hHashOuter, (const BYTE*)opad.c_str(), (DWORD)opad.size(), 0);
    CryptHashData(hHashOuter, innerHash, 32, 0);

    BYTE hmac[32];
    DWORD hmacLen = 32;
    if (!CryptGetHashParam(hHashOuter, HP_HASHVAL, hmac, &hmacLen, 0))
    {
        CryptDestroyHash(hHashOuter);
        CryptReleaseContext(hProv, 0);
        return "";
    }

    CryptDestroyHash(hHashOuter);
    CryptReleaseContext(hProv, 0);

    // To hex
    static const char hex[] = "0123456789abcdef";
    std::string hexSig;
    for (int i = 0; i < 32; i++)
    {
        hexSig += hex[(hmac[i] >> 4) & 0xf];
        hexSig += hex[hmac[i] & 0xf];
    }
    return hexSig;
}
#else
namespace
{
size_t curlWriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    const size_t totalSize = size * nmemb;
    std::string* out = static_cast<std::string*>(userp);
    out->append(static_cast<const char*>(contents), totalSize);
    return totalSize;
}

void appendHeaderLines(struct curl_slist*& headers, const std::string& rawHeaders)
{
    size_t start = 0;
    while (start < rawHeaders.size())
    {
        size_t end = rawHeaders.find("\r\n", start);
        const size_t count = (end == std::string::npos) ? (rawHeaders.size() - start) : (end - start);
        std::string line = rawHeaders.substr(start, count);
        if (!line.empty())
            headers = curl_slist_append(headers, line.c_str());
        if (end == std::string::npos)
            break;
        start = end + 2;
    }
}
}

std::string BinanceBroker::httpGet(const std::string& url) const
{
    return httpGet(url, "");
}

std::string BinanceBroker::httpGet(const std::string& url, const std::string& extraHeaders) const
{
    static std::once_flag curlInitFlag;
    std::call_once(curlInitFlag, []() { curl_global_init(CURL_GLOBAL_DEFAULT); });

    CURL* curl = curl_easy_init();
    if (!curl)
        return "";

    std::string response;
    struct curl_slist* headers = nullptr;
    appendHeaderLines(headers, extraHeaders);

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);
    if (headers)
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    const CURLcode rc = curl_easy_perform(curl);
    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

    if (headers)
        curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (rc != CURLE_OK || httpCode >= 400)
        return "";
    return response;
}

bool BinanceBroker::fetchWalletBalances(double& coin, double& cash) const
{
    if (m_config.apiKey.empty() || m_config.apiSecret.empty())
    {
        Debug::Log("fetchWalletBalances: missing API key/secret");
        return false;
    }

    const auto assets = splitSymbolAssets();
    if (assets.first.empty() || assets.second.empty())
    {
        Debug::Log("fetchWalletBalances: could not split symbol assets from " + m_config.symbol);
        return false;
    }

    const long long ts = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    std::string queryString = "timestamp=" + std::to_string(ts);
    std::string signature = signRequest(queryString);
    if (signature.empty())
    {
        Debug::Log("fetchWalletBalances: request signature generation failed");
        return false;
    }

    std::string url = "https://api.binance.com/api/v3/account?" + queryString + "&signature=" + signature;
    std::string headers = "X-MBX-APIKEY: " + m_config.apiKey + "\r\n";
    std::string resp = httpGet(url, headers);
    if (resp.empty())
    {
        Debug::Log("fetchWalletBalances: /api/v3/account returned empty response");
        return false;
    }
    if (resp.find("\"code\"") != std::string::npos || resp.find("\"msg\"") != std::string::npos)
    {
        Debug::Log("fetchWalletBalances: Binance error payload -> " + resp.substr(0, 300));
    }

    auto extractFree = [&](const std::string& asset) -> double {
        std::string marker = "\"asset\":\"" + asset + "\"";
        size_t pos = resp.find(marker);
        if (pos == std::string::npos)
            return -1.0;
        size_t freePos = resp.find("\"free\":\"", pos);
        if (freePos == std::string::npos)
            return -1.0;
        freePos += 8; // len("\"free\":\"")
        size_t end = resp.find("\"", freePos);
        if (end == std::string::npos || end <= freePos)
            return -1.0;
        return std::strtod(resp.substr(freePos, end - freePos).c_str(), nullptr);
    };

    const double coinVal = extractFree(assets.first);
    const double cashVal = extractFree(assets.second);
    if (coinVal < 0.0 || cashVal < 0.0)
    {
        Debug::Log("fetchWalletBalances: failed to parse balances for assets " + assets.first + "/" + assets.second);
        return false;
    }

    coin = coinVal;
    cash = cashVal;
    Debug::Log("fetchWalletBalances: success | " + assets.first + "=" + std::to_string(coin)
        + " | " + assets.second + "=" + std::to_string(cash));
    return true;
}

std::string BinanceBroker::formatCandleTime(long long openTimeMs) const
{
    const long long closeTimeMs = openTimeMs + candleIntervalMs();
    std::time_t tt = (std::time_t)(closeTimeMs / 1000LL);
    std::tm tmVal{};
    localtime_r(&tt, &tmVal);
    std::ostringstream ss;
    ss << std::put_time(&tmVal, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

long long BinanceBroker::candleIntervalMs() const
{
    switch (m_config.candleType)
    {
    case OHLC::m1:  return 60LL * 1000LL;
    case OHLC::m5:  return 5LL * 60LL * 1000LL;
    case OHLC::m15: return 15LL * 60LL * 1000LL;
    case OHLC::m30: return 30LL * 60LL * 1000LL;
    case OHLC::h1:  return 60LL * 60LL * 1000LL;
    case OHLC::h2:  return 2LL * 60LL * 60LL * 1000LL;
    case OHLC::h4:  return 4LL * 60LL * 60LL * 1000LL;
    case OHLC::h12: return 12LL * 60LL * 60LL * 1000LL;
    case OHLC::d1:  return 24LL * 60LL * 60LL * 1000LL;
    case OHLC::w1:  return 7LL * 24LL * 60LL * 60LL * 1000LL;
    default:        return 15LL * 60LL * 1000LL;
    }
}

std::pair<std::string, std::string> BinanceBroker::splitSymbolAssets() const
{
    static const char* knownQuotes[] = { "USDT", "BUSD", "USDC", "BTC", "ETH", "BNB", "TRY", "EUR" };
    for (const char* quote : knownQuotes)
    {
        const std::string q(quote);
        if (m_config.symbol.size() > q.size() &&
            m_config.symbol.compare(m_config.symbol.size() - q.size(), q.size(), q) == 0)
        {
            return { m_config.symbol.substr(0, m_config.symbol.size() - q.size()), q };
        }
    }
    return { "", "" };
}

std::string BinanceBroker::httpPost(const std::string& url, const std::string& queryString)
{
    static std::once_flag curlInitFlag;
    std::call_once(curlInitFlag, []() { curl_global_init(CURL_GLOBAL_DEFAULT); });

    CURL* curl = curl_easy_init();
    if (!curl)
        return "";

    std::string response;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
    headers = curl_slist_append(headers, ("X-MBX-APIKEY: " + m_config.apiKey).c_str());

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, queryString.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)queryString.size());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);

    const CURLcode rc = curl_easy_perform(curl);
    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (rc != CURLE_OK || httpCode >= 400)
        return "";
    return response;
}

std::string BinanceBroker::signRequest(const std::string& queryString) const
{
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hashLen = 0;
    const unsigned char* result = HMAC(
        EVP_sha256(),
        m_config.apiSecret.data(),
        (int)m_config.apiSecret.size(),
        reinterpret_cast<const unsigned char*>(queryString.data()),
        queryString.size(),
        hash,
        &hashLen);
    if (!result || hashLen == 0)
        return "";

    static const char hex[] = "0123456789abcdef";
    std::string hexSig;
    hexSig.reserve(hashLen * 2);
    for (unsigned int i = 0; i < hashLen; ++i)
    {
        hexSig += hex[(hash[i] >> 4) & 0x0F];
        hexSig += hex[hash[i] & 0x0F];
    }
    return hexSig;
}
#endif
