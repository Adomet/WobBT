#pragma once
#include "Strategy.h"
#include "Indicator.h"

class MyStratV3 : public Strategy
{
public:
    MyStratV3(OHLC* data, std::vector<int> params) : Strategy(data, params) {};
    ~MyStratV3() = default;

    Indicator* strend = nullptr;
    Indicator* bull_rsi = nullptr;
    Indicator* bull_tema = nullptr;
    Indicator* bear_rsi = nullptr;
    Indicator* bear_tema = nullptr;

    double bull_rsi_high = 75;
    double bull_rsi_low = 25;
    double bull_tema_high = 30;
    double bull_tema_low = 30;

    double bear_rsi_high = 75;
    double bear_rsi_low = 25;
    double bear_tema_high = 30;
    double bear_tema_low = 30;

    double takeprofit = 0.05;
    double stopLossPct = 0.05;

    void init()
    {
        strend = addIndicator<SuperTrend>(m_Data, param(0, 3), param(1, 200) / 100.0);
        bull_rsi = addIndicator<RSI>(m_Data, param(2, 21));
        bull_rsi_high = param(3, 75);
        bull_rsi_low = param(4, 25);

        bull_tema = addIndicator<TEMA>(m_Data, param(5, 51));
        bull_tema_high = param(6, 30);
        bull_tema_low = param(7, 30);


        bear_rsi = addIndicator<RSI>(m_Data, param(8, 2));
        bear_rsi_high = param(9, 80);
        bear_rsi_low = param(10, 20);

        bear_tema = addIndicator<TEMA>(m_Data, param(11, 51));
        bear_tema_high = param(12, 30);
        bear_tema_low = param(13, 30);

        takeprofit = param(14, 50) / 1000.0;
        stopLossPct = param(15, 50) / 1000.0;
    }

    void next(int candleIndex)
    {
        if (candleIndex < 1) return;

        double close = m_Data->close[candleIndex];
        double buyPrice = getBuyPrice();
        double strd = strend->line[candleIndex];
        bool isbull = (strd < close);

        if (isbull)
        {

            double bull_rsiNow = bull_rsi->line[candleIndex];
            double bull_temaNow = bull_tema->line[candleIndex];

            double bull_diff_tema_heigh = bull_temaNow + (bull_temaNow * bull_tema_high/100000);
            double bull_diff_tema_low = bull_temaNow - (bull_temaNow * bull_tema_low/10000);

    
            bool bull_rsiBuy  = bull_rsiNow <= bull_rsi_low;
            bool bull_rsiSell = bull_rsiNow >= bull_rsi_high;

            bool bull_temaBuy = close <= bull_diff_tema_low;
            bool bull_temaSell = close >= bull_diff_tema_heigh;
    
            if (bull_rsiBuy && bull_temaBuy)
                Orderer(candleIndex, true, "Bull_BUY");

            if (bull_rsiSell && bull_temaSell)
                Orderer(candleIndex, false, "Bull_SELL");
        }
        else
        {
            double bear_rsiNow = bear_rsi->line[candleIndex];
            double bear_temaNow = bear_tema->line[candleIndex];

            double bear_diff_tema_heigh = bear_temaNow + (bear_temaNow * bear_tema_high/10000);
            double bear_diff_tema_low = bear_temaNow - (bear_temaNow * bear_tema_low/10000);
    
            bool bear_rsiBuy  = bear_rsiNow <= bear_rsi_low;
            bool bear_rsiSell = bear_rsiNow >= bear_rsi_high;
    
            bool bear_temaBuy  = close <= bear_diff_tema_low;
            bool bear_temaSell = close >= bear_diff_tema_heigh;
    
            if (bear_rsiBuy && bear_temaBuy)
                Orderer(candleIndex, true, "Bear_BUY");

            if (bear_rsiSell && bear_temaSell)
                Orderer(candleIndex, false, "Bear_SELL");

        }


        bool isTakeProfit = (close >= buyPrice * (1.0 + takeprofit));
        if (isTakeProfit)
            Orderer(candleIndex, false, "Bear_TAKE PROFIT");

        bool isStopLoss = (close <= buyPrice * (1.0 - stopLossPct));
        if (isStopLoss)
            Orderer(candleIndex, false, "STOPPED");
    }
};
