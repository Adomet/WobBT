#include <stdio.h>
#include <iostream>
#include <chrono>
#include <ctime>
#include <algorithm>
#include <map>

#include "WobBTApp.h"
#include "Cerebro.h"

// strats to use at backtest
#include "Strats.h"

template <class T>
float rundata(OHLC* data, std::vector<int> params)
{
    T mystrat(data, params);
    Cerebro cerebro(&mystrat);
    cerebro.setCommissons(0.001);
    cerebro.setStartCash(1000);
    // Get result and print result
    float ret = cerebro.run();

    Debug::Log(paramStr(params) + ":::" + std::to_string(ret));

    return ret;
}

template <class T>
std::vector<int>  optimizeStrat(OHLC* data, std::vector<int> oldparams)
{
    std::vector<int> newparams = OptRunData<T>(data, oldparams);
    if (newparams == oldparams)
        return oldparams;
    else
        return optimizeStrat<T>(data, newparams);
}

template <class T>
std::vector<int>  OptRunData(OHLC* data, std::vector<int> oldparams)
{
    Debug::Log("Optimizing...");
    Debug::Log(paramStr(oldparams));

    std::vector<int> params = oldparams;

    for (int i = 0; i < params.size(); i++)
    {
        if (params[i] == -1)
            continue;

        Timer timer("param search");
        int step = std::max((params[i] / 100), 1);
        int scan_range = 31;
        int diff = step * scan_range;
        int low = params[i] - diff - step;
        int high = params[i] + diff + step;
        low = std::max(low, 1);
        high = std::max(high, 1);


        float best_value = 0;
        int   best_param = -1;
        for (int p = low; p <= high; p += step)
        {
            params[i] = p;

            float ret = rundata<T>(data, params);

            if (best_value < ret)
            {
                best_value = ret;
                best_param = p;
            }
        }

        params[i] = best_param;
        Debug::Log("Best:" + paramStr(params) + "::" + std::to_string(best_value));
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

    std::string initDate  = "2020-10-01";
    std::string stdDate   = "2021-01-19";
    std::string startDate = "2022-09-01";

    OHLC data = OHLC::getData("AVAX", "USDT", OHLC::CANDLE_TYPE::m15, startDate);

    Timer timer("All");

    std::vector<float> results;

    results.push_back(rundata<MyStratV1>(&data, optimizeStrat<MyStratV1>(&data, { 14,14,14,14,28,14,14,2,271 })));
    //results.push_back(rundata<MyStratV1>(&data, {14,14,14,14,28,14,14,2,271}));

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
// Plotting => Implot maybe pbplots
// GPU => CUDA / Compute shader donno whitch is faster ?
// Template    => Walnut
// Render API  => Vulkan
// BacktestLib => my lib

// TODO : 
// ==> Fist backtest (no graph) single Thread just return stats
// Add some indicators and analyzers
// backtest with a graph some how
// Multi thread backtest
// GPU backtest
// class todo indicators,  analyzers, ohlc candles, cerebro lines,
// Interactive backtesting!!!
// make best backtest library possiable so peaple dont fucking use 384923 of them

// wanted result: Backtest Library that looking good with advanced strat cutomization and fastest backtester lets goooo

// ===================== Fist backtest (no graph) single Thread just return stats ====================================
// x read csv and get data  
// ==> write your strat based on OHLC update data
// Create Strat Object and add to backtester (need a good name)
// make Init start update end funciton of Strat
// read analyzers from strat


// ===================== Ideas ==================================== 
// slider based realtime strat result update
// Custom Ploting api
// Advanced Cutomizeablty

//====================================================================================================================================================================================

// Create and add data to cerebro and Run
// Add data,Strategy,startCash,Analyzers,Commsions to cerebro
// Create a WobBTApp with some arguments when needed
// App will take a backtester at the end of operations return a backtester with data
// After all opereations done press a ImGui button to save backtester params to a log file (log.txt) i donno