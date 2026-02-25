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
        }
    }

    OHLC(std::vector<double> open, std::vector<double> high, std::vector<double> low, std::vector<double> close, std::vector<double> volume, CANDLE_TYPE candleType);
    ~OHLC();

    static OHLC CSV2OHLC(std::string filepath, std::string tradeCoin, std::string stableCoin, CANDLE_TYPE candleType, std::string backtestDate, bool reget);
    static std::string OHLC::getDataFilePath(std::string tradeCoin, std::string stableCoin, OHLC::CANDLE_TYPE candleType, std::string backtestDate);
    static OHLC OHLC::getData(std::string tradeCoin, std::string stableCoin, OHLC::CANDLE_TYPE type, std::string backtestDate,bool reget);
    static int candlesPerDay(CANDLE_TYPE type);
    OHLC slice(size_t start, size_t end) const;

public:
    std::vector<double> open;
    std::vector<double> high;
    std::vector<double> low;
    std::vector<double> close;
    std::vector<double> volume;
    CANDLE_TYPE m_CandleType;
};


