#include <stdio.h>
#include <iostream>
#include <chrono>
#include <ctime>
#include <algorithm>
#include <map>
#include <future>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <unordered_map>
#include "Cerebro.h"
#include "Analyzers.h"
#include "Strats.h"
#include "Binance.h"

enum RetVal
{
    Ado, All, Return,Cash, TradeCount , Sharpe
};

static std::string trim(const std::string& s)
{
    const std::string ws = " \t\r\n";
    const size_t start = s.find_first_not_of(ws);
    if (start == std::string::npos) return "";
    const size_t end = s.find_last_not_of(ws);
    return s.substr(start, end - start + 1);
}

static std::unordered_map<std::string, std::string> loadDotEnv(const std::string& path)
{
    std::unordered_map<std::string, std::string> envMap;
    std::ifstream file(path);
    if (!file.is_open())
        return envMap;

    std::string line;
    while (std::getline(file, line))
    {
        std::string s = trim(line);
        if (s.empty() || s[0] == '#')
            continue;
        if (s.rfind("export ", 0) == 0)
            s = trim(s.substr(7));

        const size_t eq = s.find('=');
        if (eq == std::string::npos || eq == 0)
            continue;

        std::string key = trim(s.substr(0, eq));
        std::string val = trim(s.substr(eq + 1));
        if (val.size() >= 2)
        {
            const bool quoted = (val.front() == '"' && val.back() == '"') ||
                                (val.front() == '\'' && val.back() == '\'');
            if (quoted)
                val = val.substr(1, val.size() - 2);
        }
        envMap[key] = val;
    }
    return envMap;
}

static void addDefaultAnalyzers(Cerebro& cerebro, Strategy& strat)
{
    cerebro.addAnalyzer(new TotalClosed(strat));
    cerebro.addAnalyzer(new TotalWon(strat));
    cerebro.addAnalyzer(new TotalLost(strat));
    cerebro.addAnalyzer(new ReturnPNL(strat));
    cerebro.addAnalyzer(new WinStreaks(strat));
    cerebro.addAnalyzer(new LoseStreaks(strat));
    cerebro.addAnalyzer(new WinRate(strat));
    cerebro.addAnalyzer(new MaxDD(strat));
    cerebro.addAnalyzer(new DrawDown(strat));
    cerebro.addAnalyzer(new AvgDD(strat));
    cerebro.addAnalyzer(new Expectancy(strat));
    cerebro.addAnalyzer(new ProfitFactor(strat));
    cerebro.addAnalyzer(new SQN(strat));
    cerebro.addAnalyzer(new SharpeRatio(strat));
}

static double computeScore(const CerebroResult& r, RetVal retval)
{
    double growth = r.getResult<ReturnPNL>()/1000.0;
    double maxDD = r.getResult<MaxDD>();
    double avgDD = r.getResult<AvgDD>();
    double sqn = r.getResult<SQN>();
    double expectancy = r.getResult<Expectancy>();
    double tradeCount = r.getResult<TotalClosed>();
    double winRate = r.getResult<WinRate>();
    double sharpe = r.getResult<SharpeRatio>();
    double profitFactor = r.getResult<ProfitFactor>();
    double res = 0;
    switch (retval)
    {
    case Ado:
        res = (std::sqrt(growth)  * sqn * sharpe) / ((avgDD * std::sqrt(maxDD) ) + 1.0);
        break;
    case All:
        res = std::sqrt(growth)* profitFactor * expectancy * sharpe * sqn / ((avgDD * maxDD) + 1.0);
        break;
    case Return:
        res = growth / ((avgDD * maxDD) + 1.0);
        break;
    case Cash:
        res = r.getResult<ReturnPNL>();
        break;
    case TradeCount:
        res = tradeCount * tradeCount* tradeCount* std::sqrt(growth);
        break;
    case Sharpe:
        res = sharpe;
    default:
        break;
    }
    return std::isfinite(res) ? res : 0.0;
}



template <class T>
double walkForward(int trainDays, int testDays, OHLC* data, std::vector<int> params, RetVal retVal = RetVal::Return)
{
    const int cpd = OHLC::candlesPerDay(data->m_CandleType);
    if (cpd <= 0) return 0;

    const int trainCandles = trainDays * cpd;
    const int testCandles = testDays * cpd;
    const size_t totalCandles = data->close.size();

    if ((size_t)(trainCandles + testCandles) > totalCandles)
    {
        Debug::Log("walkForward: not enough data (need " + std::to_string(trainCandles + testCandles) + " candles)");
        return 0;
    }

    double totalScore = 0;
    int blockCount = 0;
    // Data: index 0 = oldest, last = most recent. Test must be most recent.
    // Block 0: test [N-tc, N], block 1: test [N-2*tc, N-tc], etc.
    size_t testStart = totalCandles - testCandles;

    while (testStart >= trainCandles)
    {
        Debug::Log("=================== Block " + std::to_string(blockCount) + " ==========================");

        OHLC trainData = data->slice(0, testStart);
        OHLC testData = data->slice(testStart, testStart + testCandles); 

        std::vector<int> bestParams = optimizeStrat<T>(params, &trainData, retVal);
        double trainScore = run<T>(bestParams, &trainData, false, false, false, retVal);
        double oosScore = run<T>(bestParams, &testData, false, true, false, retVal);

        Debug::Log("Train score: " + std::to_string(trainScore) + " | Test (OOS) score: " + std::to_string(oosScore));

        totalScore += oosScore;
        blockCount++;
        params = bestParams;
        if (testStart < testCandles) break;
        testStart -= testCandles;
    }

    return blockCount > 0 ? totalScore / blockCount : 0;
}

template <class T>
double trainTest(int trainDays, int testDays, OHLC* data, std::vector<int> params, RetVal retVal = RetVal::Return)
{
    const int cpd = OHLC::candlesPerDay(data->m_CandleType);
    if (cpd <= 0) return 0;

    const int trainCandles = trainDays * cpd;
    const int testCandles = testDays * cpd;
    const size_t totalCandles = data->close.size();

    if ((size_t)(trainCandles + testCandles) > totalCandles)
    {
        Debug::Log("walkForward: not enough data (need " + std::to_string(trainCandles + testCandles) + " candles)");
        return 0;
    }

    size_t testEnd = totalCandles - testCandles;
    size_t trainEnd = testEnd - trainCandles;

    OHLC trainData = data->slice(trainEnd, testEnd);
    OHLC testData = data->slice(testEnd, totalCandles);

    std::vector<int> bestParams = optimizeStrat<T>(params, &trainData, retVal);
    double trainScore = run<T>(bestParams, &trainData, false, false, false, Cash);
    double oosScore = run<T>(bestParams, &testData, false, false, false, Cash);

    Debug::Log("Train score: " + std::to_string(trainScore) + " | Test (OOS) score: " + std::to_string(oosScore));
    return oosScore;
}

template <class T>
double run(std::vector<int> params, OHLC* data, bool optimize, bool showAnalysis, bool showPlot, RetVal retval = RetVal::Cash)
{
    if (optimize)
    {
        return run<T>(optimizeStrat<T>(params, data, retval), data, false, showAnalysis, showPlot, retval);
    }

    T mystrat(data, params);
    Cerebro cerebro(&mystrat);
    addDefaultAnalyzers(cerebro, mystrat);

    CerebroResult result = cerebro.run();
    double res = computeScore(result, retval);

    Debug::Log(paramStr(params) + "::" + std::to_string(res) + " ::: Result");
    if (showAnalysis)
        result.print();

    if (showPlot)
    {
        try
        {
            mystrat.Plot(false);
        }
        catch (const std::exception& e)
        {
            Debug::Log("Plot error: " + std::string(e.what()));
        }
    }

    mystrat.deleteElements();
    return res;
}



static std::mutex s_runData_mutex;
template <class T>
void rundataAsync(std::vector<int> params, OHLC* data, int i, std::vector<CerebroResult>* results, RetVal retval)
{
    T mystrat(data, params);
    Cerebro cerebro(&mystrat);
    addDefaultAnalyzers(cerebro, mystrat);

    CerebroResult result = cerebro.run();
    result.scan_param = params[i];
    result.score = computeScore(result, retval);

    std::lock_guard<std::mutex> lock(s_runData_mutex);
    results->push_back(result);

    mystrat.deleteElements();
}

template <class T>
std::vector<int>  optimizeStrat(std::vector<int> oldparams, OHLC* data, RetVal retval = Return, int scan_range = 16, bool singleStep = false)
{
    std::vector<int> newparams = OptRunData<T>(data, oldparams, scan_range, singleStep, retval);
    if (newparams == oldparams)
    {
        if (scan_range > 30)
        {
            if (!singleStep)
                return optimizeStrat<T>(newparams, data, retval, scan_range, true);
            else
                return newparams;
        }
        else
        {
            return optimizeStrat<T>(newparams, data, retval,scan_range * 2);
        }
    }
    else
    {
        return optimizeStrat<T>(newparams, data, retval, scan_range);
    }
}

template <class T>
std::vector<int>  OptRunData(OHLC* data, std::vector<int> oldparams, int scan_range,bool singleStep, RetVal retval)
{
    Debug::Log("Optimizing...");
    Debug::Log("scan_range...:" + std::to_string(scan_range));
    Debug::Log(paramStr(oldparams));

    std::vector<int> params = oldparams;

    for (int i = 0; i < params.size(); i++)
    {
        if (params[i] == -1)
            continue;

        int step = std::max(params[i] / 100, 1);
        if(singleStep) step = 1;
        int diff = step * scan_range;
        int low  = params[i] - diff;
        int high = params[i] + diff;

        std::vector<CerebroResult> results;
        std::vector<std::future<void>> m_futures;

        for (int p = low; p <= high; p += step)
        {
            params[i] = std::max(p, 1);
            m_futures.push_back(std::async(std::launch::async, rundataAsync<T>, params, data, i, &results, retval));
        }

        for (size_t f = 0; f < m_futures.size(); f++)
        {
            m_futures[f].wait();
        }

        int best_param = -1;
        double best_value = 0;

        for (const auto& r : results)
        {
            if (best_value < r.score)
            {
                best_value = r.score;
                best_param = r.scan_param;
            }
            else if (best_value == r.score && best_param > r.scan_param)
            {
                best_param = r.scan_param;
            }
        }

        params[i] = best_param;
        Debug::Log(paramStr(params) + "::" + std::to_string(best_value) + " :Best");
    }

    return params;
}

std::string paramStr(std::vector<int>& params)
{
    std::string msg = "{";
    for (size_t i = 0; i < params.size(); i++)
    {
        if (i > 0) msg += ",";
        msg += std::to_string(params[i]);
    }
    msg += "}";
    return msg;
}

void printHeader()
{
    Debug::Log("------------------------------------------------------------------------------------------");
    Debug::Log("------------------------------------- WOB BACKTESTER -------------------------------------");
    Debug::Log("------------------------------------------------------------------------------------------");
    Debug::Log("");
}

template <class T>
double runLive(std::vector<int> params)
{
    BinanceConfig cfg;
    const auto dotEnv = loadDotEnv(".env");

    auto fromDotEnv = [&](const std::string& key) -> std::string {
        const auto it = dotEnv.find(key);
        return it == dotEnv.end() ? "" : it->second;
    };

    const char* apiKey = std::getenv("BINANCE_API_KEY");
    const char* apiSecret = std::getenv("BINANCE_API_SECRET");
    cfg.apiKey = fromDotEnv("BINANCE_API_KEY");
    cfg.apiSecret = fromDotEnv("BINANCE_API_SECRET");
    if (apiKey && *apiKey) cfg.apiKey = apiKey;       // env var overrides .env
    if (apiSecret && *apiSecret) cfg.apiSecret = apiSecret;
    if (cfg.apiKey.empty() || cfg.apiSecret.empty())
    {
        Debug::Log("BINANCE_API_KEY / BINANCE_API_SECRET not set in .env or env vars. Running in simulation mode (no real orders).");
    }

    cfg.symbol = "AVAXUSDT";
    cfg.candleType = OHLC::m15;
    cfg.warmupCandles = 500;
    cfg.pollIntervalSec = 10;

    BinanceBroker bb(cfg);
    T mystrat(bb.getOHLC(), params);
    Cerebro cerebro(&mystrat);
    bb.runLive(cerebro);
    Debug::Log(paramStr(params) + " ::: Live");
    mystrat.deleteElements();
    return 0;
}

int runWobBT(int argc, char** argv)
{
    printHeader();
    //2020-09-01
    //2022-06-11

    OHLC data = OHLC::getData("AVAX", "USDT", OHLC::CANDLE_TYPE::m15, "2020-09-01", false);
    //Timer timer("All");
    
    //run<MyStratV1>({ 255, 993, 149, 23, 408, 731, 1383, 16, 566, 337, 125, 144, 180, 789, 524, 242, 164, 69, 38, 68 }, &data, false, true, true, All);
    //run<MyStratV1>({ 265,985,152,23,472,731,1539,19,573,312,123,142,171,790,524,242,123,40,36,59 }, &data, false, true, true, All);
    //run<MyStratV1>({ 265,989,149,23,408,741,1519,11,623,309,124,141,166,790,523,261,79,47,44,58 }, &data, true, true, true, All);
    //run<MyStratV1>({ 263,930,150,24,294,765,1382,20,570,330,126,139,204,1135,533,220,131,77,36,69 }, &data, true, true, true, Cash);

    //trainTest<MyStratV1>(1000, 360, &data, { 265,985,152,23,472,731,1539,19,573,312,123,142,171,790,524,242,123,40,36,59 }, All);
    //walkForward<MyStratV1>(720, 360, &data, { 260, 960, 149, 23, 313, 731, 1382, 16, 568, 341, 125, 148, 165, 786, 524, 204, 169, 35, 38, 69 }, Ado);
    // 
    // 

    //runLive<MyStratV1>({ 265,985,152,23,472,731,1539,19,573,312,123,142,171,790,524,242,123,40,36,59 });

    
    return 0;
}

int main(int argc, char** argv)
{
    return runWobBT(argc,argv);
}