#include "OHLC.h"
#include <algorithm>
#include <ctime>

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

OHLC  OHLC::CSV2OHLC(std::string filepath, std::string tradeCoin, std::string stableCoin, CANDLE_TYPE candleType, std::string backtestDate, bool reget)
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
    printf("%s", msg.c_str());

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
    printf("%s", msg2.c_str());
    printf("\n");
    printf("------------------------------------------------------------------------------------------");
    printf("\n");

    if (candleCount <= 0 || reget)
    {
        printf("Get Data !!!");
        printf("\n");

        std::string pyCall =
#ifdef _WIN32
            "py get_data.py ";
#else
            "python3 get_data.py ";
#endif
        pyCall += backtestDate + " " + tradeCoin;
        int retCode = system(pyCall.c_str());
        (void)retCode;

        return CSV2OHLC(filepath,tradeCoin,stableCoin, candleType,backtestDate,false);
    }

    return OHLC(open, high, low, close, volume, candleType);
}


std::string OHLC::getDataFilePath(std::string tradeCoin, std::string stableCoin, OHLC::CANDLE_TYPE candleType, std::string backtestDate)
{
    // Get date
    time_t now = time(0);
    tm tPtr;
    now += (86400);
#ifdef _WIN32
    localtime_s(&tPtr, &now);
#else
    localtime_r(&now, &tPtr);
#endif
    size_t n = 2;
    std::string yr_s = std::to_string(tPtr.tm_year + 1900);
    std::string mon_s = std::to_string(tPtr.tm_mon+1);




    int p = n - std::min(n, mon_s.size());
    mon_s = std::string(p, '0').append(mon_s);
    std::string mday_s = std::to_string(tPtr.tm_mday);
    p = n - std::min(n, mday_s.size());
    mday_s = std::string(p, '0').append(mday_s);

    std::string path = "";
    path += "./data/" + tradeCoin + "-" + stableCoin + "_" + OHLC::candleString(candleType) + "_" + backtestDate + "=" + yr_s + "-" + mon_s + "-" + mday_s + ".csv";

    return path;
}

OHLC OHLC::getData(std::string tradeCoin, std::string stableCoin, OHLC::CANDLE_TYPE type, std::string backtestDate,bool reget)
{
    std::string path = OHLC::getDataFilePath(tradeCoin, stableCoin, type, backtestDate);
    OHLC data = OHLC::CSV2OHLC(path,tradeCoin,stableCoin, type, backtestDate, reget);
    return data;
}

int OHLC::candlesPerDay(CANDLE_TYPE type)
{
    switch (type)
    {
    case m1:  return 1440;
    case m5:  return 288;
    case m15: return 96;
    case m30: return 48;
    case h1:  return 24;
    case h2:  return 12;
    case h4:  return 6;
    case h12: return 2;
    case d1:  return 1;
    case w1:  return 1;  // 1 block = 1 week
    default:  return 96;
    }
}

OHLC OHLC::slice(size_t start, size_t end) const
{
    size_t n = close.size();
    end = std::min(end, n);
    if (start >= end) return OHLC({}, {}, {}, {}, {}, m_CandleType);

    return OHLC(
        std::vector<double>(open.begin() + start, open.begin() + end),
        std::vector<double>(high.begin() + start, high.begin() + end),
        std::vector<double>(low.begin() + start, low.begin() + end),
        std::vector<double>(close.begin() + start, close.begin() + end),
        std::vector<double>(volume.begin() + start, volume.begin() + end),
        m_CandleType
    );
}