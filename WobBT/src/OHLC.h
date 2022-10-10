#pragma once
#include <iostream>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <sstream>


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


std::string candleString(enum CANDLE_TYPE type)
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



class OHLC
{
public:
    OHLC(std::vector<float> open, std::vector<float> high, std::vector<float> low, std::vector<float> close, std::vector<float> volume, CANDLE_TYPE candleType);
    ~OHLC();

    OHLC CSV2OHLC(std::string filepath);


public:
    std::vector<float> open;
    std::vector<float> high;
    std::vector<float> low;
    std::vector<float> close;
    std::vector<float> volume;
    CANDLE_TYPE m_CandleType;
};

OHLC::OHLC(std::vector<float> open, std::vector<float>high, std::vector<float> low, std::vector<float> close, std::vector<float> volume, CANDLE_TYPE candleType)
{
    OHLC::open = open;
    OHLC::high = high;
    OHLC::low = low;
    OHLC::close = close;
    OHLC::volume = volume;
    OHLC::m_CandleType = candleType;

}

OHLC::~OHLC()
{}

OHLC CSV2OHLC(std::string filepath, CANDLE_TYPE candleType)
{
    // Add date
    std::vector<float> open;
    std::vector<float> high;
    std::vector<float> low;
    std::vector<float> close;
    std::vector<float> volume;

    //read filepath and construct data and return

    std::ifstream inputfile;

    std::string msg = "Fetching data of:" + filepath;
    printf(msg.c_str());

    inputfile.open(filepath);
    std::string line = "";
    while (std::getline(inputfile, line))
    {
        std::stringstream inputString(line);

        std::string datestr;
        std::string openstr;
        std::string highstr;
        std::string lowstr;
        std::string closestr;
        std::string volumestr;

        std::getline(inputString, datestr, ',');
        std::getline(inputString, openstr, ',');
        std::getline(inputString, highstr, ',');
        std::getline(inputString, lowstr, ',');
        std::getline(inputString, closestr, ',');
        std::getline(inputString, volumestr, ',');

        open.push_back(std::stof(openstr));
        high.push_back(std::stof(highstr));
        low.push_back(std::stof(lowstr));
        close.push_back(std::stof(closestr));
        volume.push_back(std::stof(volumestr));

        //printf("\n");
        //printf(line.c_str());

        continue;
    }

    size_t candleCount = close.size();
    std::string msg2 = "Candle count:" + std::to_string(candleCount);
    printf("\n");
    printf(msg2.c_str());

    if (candleCount <= 0)
    {
        printf("\n");
        printf("No data Found!!!");
    }

    return OHLC(open, high, low, close, volume, candleType);
}



