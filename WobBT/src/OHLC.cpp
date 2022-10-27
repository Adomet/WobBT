#include "OHLC.h"

OHLC::OHLC(std::vector<double> open, std::vector<double>high, std::vector<double> low, std::vector<double> close, std::vector<double> volume, CANDLE_TYPE candleType)
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

OHLC  OHLC::CSV2OHLC(std::string filepath, CANDLE_TYPE candleType, std::string backtestDate, bool reget)
{

    // Add date
    std::vector<double> open;
    std::vector<double> high;
    std::vector<double> low;
    std::vector<double> close;
    std::vector<double> volume;

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
    printf("\n");
    printf("------------------------------------------------------------------------------------------");
    printf("\n");

    if (candleCount <= 0 || reget)
    {
        printf("Get Data !!!");
        printf("\n");

        std::string pyCall = "py get_data.py " + backtestDate;
        int retCode = system(pyCall.c_str());

        return CSV2OHLC(filepath, candleType,backtestDate,false);
    }

    return OHLC(open, high, low, close, volume, candleType);
}


std::string OHLC::getDataFilePath(std::string tradeCoin, std::string stableCoin, OHLC::CANDLE_TYPE candleType, std::string backtestDate)
{
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
    path += "./data/" + tradeCoin + "-" + stableCoin + "_" + OHLC::candleString(candleType) + "_" + backtestDate + "=" + yr_s + "-" + mon_s + "-" + mday_s + ".csv";

    return path;
}

OHLC OHLC::getData(std::string tradeCoin, std::string stableCoin, OHLC::CANDLE_TYPE type, std::string backtestDate,bool reget)
{
    // Get Data YYYY-MM-DD
    std::string path = OHLC::getDataFilePath(tradeCoin, stableCoin, type, backtestDate);
    OHLC data = OHLC::CSV2OHLC(path, type, backtestDate, reget);
    return data;
}