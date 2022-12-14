#include <stdio.h>
#include <iostream>
#include <chrono>
#include <ctime>
#include <algorithm>
#include <map>
#include <future>
#include "WobBTApp.h"
#include "Cerebro.h"
#include "Analyzers.h"
// strats to use at backtest
#include "Strats.h"

struct ReturnPayload
{
public:
    int param = 0;
    double ret = 0;
};

template <class T>
double rundata(std::vector<int> params,OHLC* data,bool showAnalysis,bool showPlot)
{
    T mystrat(data, params);
    Cerebro cerebro(&mystrat);
    cerebro.setCommissons(0.001);
    cerebro.setStartCash(1000);
    
    ReturnPNL*    ret     = new ReturnPNL(mystrat);
    WinStreaks*   winS    = new WinStreaks(mystrat);
    LoseStreaks*  loseS   = new LoseStreaks(mystrat);
    WinRate*      winrate = new WinRate(mystrat);
    MaxDD*        maxDD   = new MaxDD(mystrat);
    SQN*          sqn     = new SQN(mystrat);

    cerebro.addAnalyzer(ret    );
    cerebro.addAnalyzer(winS   );
    cerebro.addAnalyzer(loseS  );
    cerebro.addAnalyzer(winrate);
    cerebro.addAnalyzer(maxDD  );
    cerebro.addAnalyzer(sqn    );

    // Get result and print result
    cerebro.run();

    //double res = sqrt(ret->m_Result) * sqn->m_Result * winrate->m_Result * mystrat.m_winTradeCount / maxDD->m_Result;
    double res = sqrt(ret->m_Result) * sqn->m_Result * winrate->m_Result * mystrat.m_winTradeCount / maxDD->m_Result;
    Debug::Log(paramStr(params) + "::" + std::to_string(res) + " ::: Result");

    if (showAnalysis)
        mystrat.printResults();

    if (showPlot)
        mystrat.Plot();

    mystrat.deleteElements();
    return res;
}

static std::mutex s_runData_mutex;
template <class T>
void rundataAsync(std::vector<int> params,OHLC* data,int i, std::vector<ReturnPayload>* result_Payloads)
{
    T mystrat(data, params);
    Cerebro cerebro(&mystrat);
    cerebro.setCommissons(0.001);
    cerebro.setStartCash(1000);

    ReturnPNL* ret = new ReturnPNL(mystrat);
    WinStreaks* winS = new WinStreaks(mystrat);
    LoseStreaks* loseS = new LoseStreaks(mystrat);
    WinRate* winrate = new WinRate(mystrat);
    MaxDD* maxDD = new MaxDD(mystrat);
    SQN* sqn = new SQN(mystrat);

    cerebro.addAnalyzer(ret);
    cerebro.addAnalyzer(winS);
    cerebro.addAnalyzer(loseS);
    cerebro.addAnalyzer(winrate);
    cerebro.addAnalyzer(maxDD);
    cerebro.addAnalyzer(sqn);

    // Get result and print result
    cerebro.run();

    double res = sqrt(ret->m_Result) * sqn->m_Result * winrate->m_Result * mystrat.m_winTradeCount / maxDD->m_Result;

    ReturnPayload payload;
    payload.param = params[i];
    payload.ret = res;

    std::lock_guard<std::mutex> lock(s_runData_mutex);
    result_Payloads->push_back(payload);
    //Debug::Log(paramStr(params) + ":::" + std::to_string(ret));

    mystrat.deleteElements();
}

template <class T>
std::vector<int>  optimizeStrat(std::vector<int> oldparams, OHLC* data, int scan_range = 4)
{
    std::vector<int> newparams = OptRunData<T>(data, oldparams, scan_range);
    if (newparams == oldparams)
    {
        if (scan_range > 100)
            return newparams;
        else
            return optimizeStrat<T>(newparams,data,scan_range * 2);
    }
    else
    {
        return optimizeStrat<T>(newparams, data, scan_range);
    }
}

template <class T>
std::vector<int>  OptRunData(OHLC* data, std::vector<int> oldparams, int scan_range)
{
    Debug::Log("Optimizing...");
    Debug::Log("scan_range...:" + std::to_string(scan_range));
    Debug::Log(paramStr(oldparams));

    std::vector<int> params = oldparams;

    for (int i = 0; i < params.size(); i++)
    {
        if (params[i] == -1)
            continue;

        //Timer timer("param search");
        int step = std::max(params[i] / 100, 1);
        step = 1;
        int diff = step * scan_range;
        int low  = params[i] - diff;
        int high = params[i] + diff;

        std::vector<ReturnPayload> result_Payloads;

        std::vector<std::future<void>> m_futures;

        for (int p = low; p <= high; p += step)
        {
            params[i] = std::max(p, 1);
            m_futures.push_back(std::async(std::launch::async, rundataAsync<T>, params, data, i, &result_Payloads));
        }

        for (size_t f = 0; f < m_futures.size(); f++)
        {
            m_futures[f].wait();
        }

        int best_param = -1;
        double best_value = 0;

        for (size_t x = 0; x < result_Payloads.size(); x++)
        {
            auto ret = result_Payloads[x].ret;
            auto p = result_Payloads[x].param;

            if (best_value < ret)
            {
                best_value = ret;
                best_param = p;
            }
            else if (best_value == ret && best_param > p)
            {
                best_param = p;
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
        msg += std::to_string(params[i]) + ",";
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

TA_RetCode initTaLib()
{
    auto retCode = TA_Initialize();
    if (retCode != TA_SUCCESS) {
        std::cout << "CANNOT INITIALIZE TA-LIB!" << std::endl;
    }

    return retCode;
}


int runWobBT(int argc, char** argv)
{
    printHeader();

    initTaLib();

    std::string initDate  = "2020-09-23";
    std::string stdDate   = "2021-01-17";
    std::string startDate = "2022-09-01";

    OHLC data = OHLC::getData("AVAX", "USDT", OHLC::CANDLE_TYPE::m15, initDate,false);

    Timer timer("All");

    std::vector<double> results;

    results.push_back(rundata<MyStratV1>(optimizeStrat<MyStratV1>({ 2,271,2,910,160,56,259,254,1617,19,525,348,101,175,340,1161,572,280,160,-1,-1 },&data),&data, true, false));
    //results.push_back(rundata<MyStratV1>({ 2,271,2,910,160,56,259,254,1617,19,525,348,101,175,340,1161,572,280,160,-1,-1, },&data,true,false));
    //results.push_back(rundata<MyStratV1>({2,273,2,876,159,56,347,248,1617,19,525,348,101,180,340,1156,567,280,160,-1,-1,},&data,true,false));

    auto best = std::max_element(results.begin(),results.end()); 
    Debug::Log("Best result: " + std::to_string(*best));

    // By By
    TA_Shutdown();
    return 0;
}



int main(int argc, char** argv)
{
    return runWobBT(argc,argv);
}


//====================================================================================================================================================================================
// Libraries to use
// GUI => Imgui
// TA  => ta-lib
// Plotting => Matplotlib
// Template    => Walnut
// Render API  => Vulkan
// BacktestLib => my lib

// TODO : 
// X Fist backtest (no graph) single Thread just return stats
// X Add someindicators
// X Add some analyzers
// X backtest with a graph some how
// X Multi thread backtest
// (No need) GPU backtest
// Interactive backtesting!!!

// wanted result: Backtest Library that looking good with advanced strat cutomization and fastest backtester lets goooo


// ===================== Ideas ==================================== 
// slider based realtime strat result update
// Custom Ploting api
// Advanced Cutomizeablty

//====================================================================================================================================================================================

// Create a WobBTApp with some arguments when needed
// App will take a backtester at the end of operations return a backtester with data
// After all opereations done press a ImGui button to save backtester params to a log file (log.txt) i donno