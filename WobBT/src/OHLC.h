#pragma once
#include <iostream>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <sstream>


class OHLC
{
public:
    enum CANDLE_TYPE
    {
        m1,
        m5,
        m15,
        m30,
        h1,
        h2,
        h4,
        h12,
        d1,
        w1
    };

    static  inline std::string candleString(enum CANDLE_TYPE type)
    {
        switch (type)
        {
        case m1:  return "1m";
        case m5:  return "5m";
        case m15: return "15m";
        case m30: return "30m";
        case h1:  return "1h";
        case h2:  return "2h";
        case h4:  return "4h";
        case h12: return "12h";
        case d1:  return "1d";
        case w1:  return "1w";
        default:  return "1m";
            break;
        }
    }

    OHLC(std::vector<float> open, std::vector<float> high, std::vector<float> low, std::vector<float> close, std::vector<float> volume, CANDLE_TYPE candleType);
    ~OHLC();

    static OHLC CSV2OHLC(std::string filepath, CANDLE_TYPE candleType, std::string backtestDate);
    static std::string OHLC::getDataFilePath(std::string tradeCoin, std::string stableCoin, OHLC::CANDLE_TYPE candleType, std::string backtestDate);
    static OHLC OHLC::getData(std::string tradeCoin, std::string stableCoin, OHLC::CANDLE_TYPE type, std::string backtestDate);

public:
    std::vector<float> open;
    std::vector<float> high;
    std::vector<float> low;
    std::vector<float> close;
    std::vector<float> volume;
    CANDLE_TYPE m_CandleType;
};


