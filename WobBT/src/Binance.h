#pragma once
#include "IBroker.h"
#include "OHLC.h"
#include "Cerebro.h"
#include <string>
#include <vector>
#include <atomic>
#include <thread>
#include <utility>

// API keys: set before runLive or load from config
struct BinanceConfig
{
    std::string apiKey;
    std::string apiSecret;
    std::string symbol = "AVAXUSDT";
    OHLC::CANDLE_TYPE candleType = OHLC::m15;
    int warmupCandles = 500;   // initial history for indicator warmup
    int pollIntervalSec = 30;  // how often to check for new candle
};

class BinanceBroker : public IBroker
{
public:
    explicit BinanceBroker(const BinanceConfig& config);
    ~BinanceBroker();

    void executeOrder(bool isBuy, double price) override;
    BrokerState getState() override;
    void refreshState();

    // Runs live loop: fetches data, on each new candle runs strategy through Cerebro
    void runLive(Cerebro& cerebro);

    OHLC* getOHLC() { return &m_ohlc; }

private:
    void fetchInitialKlines();
    bool fetchLatestKlines();
    void appendCandle(double o, double h, double l, double c, double v);
    void removeLastCandle();
    std::string httpGet(const std::string& url) const;
    std::string httpGet(const std::string& url, const std::string& extraHeaders) const;
    std::string httpPost(const std::string& url, const std::string& queryString);
    std::string signRequest(const std::string& queryString) const;
    bool parseKline(const char* buf, size_t len, size_t& off, double& o, double& h, double& l, double& c, double& v, long long* outOpenTime = nullptr);
    bool fetchWalletBalances(double& coin, double& cash) const;
    std::string formatCandleTime(long long openTimeMs) const;
    long long candleIntervalMs() const;
    std::pair<std::string, std::string> splitSymbolAssets() const;
    void logLiveCandles(const BrokerState& state) const;

    BinanceConfig m_config;
    OHLC m_ohlc;
    BrokerState m_cachedState;
    long long m_lastKlineTime = 0;
    long long m_lastProcessedOpenTime = 0;
    double m_lastProcessedClose = 0.0;
    std::atomic<bool> m_running{ true };
};
