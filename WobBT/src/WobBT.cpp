#include <stdio.h>
#include <iostream>
#include <chrono>
#include <ctime> 
#include "WobBTApp.h"
#include "OHLC.h"
#include "ta_libc.h"

std::string getDataFilePath(std::string tradeCoin, std::string stableCoin, CANDLE_TYPE candleType, int dayCount)
{
    std::string backtestDate = "2021-01-19";

    // Get date
    const time_t now = time(0);
    tm tPtr;
    localtime_s(&tPtr, &now);
    size_t n = 2;
    std::string yr_s = std::to_string(tPtr.tm_year + 1900);
    std::string mon_s = std::to_string(tPtr.tm_mon + 1);
    int p = n - std::min(n, mon_s.size());
    mon_s = std::string(p, '0').append(mon_s);
    std::string mday_s = std::to_string(tPtr.tm_mday + 1);
    p = n - std::min(n, mday_s.size());
    mday_s = std::string(p, '0').append(mday_s);

    std::string path = "";
    path += "./data/" + tradeCoin + "-" + stableCoin + "_" + candleString(candleType) + "_" + backtestDate + "=" + yr_s + "-" + mon_s + "-" + mday_s + ".csv";

    return path;
}


int runWobBT(int argc, char** argv)
{
    std::string tradeCoin = "AVAX";
    std::string stableCoin = "USDT";
    CANDLE_TYPE type = CANDLE_TYPE::m15;
    int dayCount = 720;
    std::string path = getDataFilePath(tradeCoin, stableCoin, type, 720);
    OHLC data = CSV2OHLC(path, type);

    //Create a WobBTApp with some arguments when needed

    // App will take a backtester at the end of operations return a backtester with data
    // After all opereations done press a ImGui button to save backtester params to a log file (log.txt) i donno


    auto retCode = TA_Initialize();
    if (retCode != TA_SUCCESS) {
        std::cout << "CANNOT INITIALIZE TA-LIB!" << std::endl;
        return -1;
    }


    if (false)
    {
        WobBTApp app(0, nullptr);
        app.Run();
    }

    return TA_Shutdown();
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