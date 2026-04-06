#pragma once
#include "Strategy.h"
#include "Indicator.h"

// Simple Golden Cross: short MA crosses above long MA = buy, below = sell
class GoldenCross : public Strategy
{
public:
    GoldenCross(OHLC* data, std::vector<int> params) : Strategy(data, params) {};
    ~GoldenCross() = default;

    Indicator* smaShort = nullptr;
    Indicator* smaLong = nullptr;
    double stopLossPct = 0.05;
    double takeProfitPct = 0.05;

    void init()
    {
        int shortPeriod = param(0, 50);
        int longPeriod = param(1, 200);
        stopLossPct = param(2, 5) / 100.0;  // param(2) = stop loss % (e.g. 5 = 5%)
        takeProfitPct = param(3, 5) / 100.0;  // param(3) = take profit % (e.g. 5 = 5%)
        smaShort = addIndicator<SMA>(m_Data, shortPeriod);
        smaLong = addIndicator<SMA>(m_Data, longPeriod);
    }

    void next(int candleIndex)
    {
        if (candleIndex < 1) return;

        double close = m_Data->close[candleIndex];
        double shortNow = smaShort->line[candleIndex];
        double longNow = smaLong->line[candleIndex];
        double shortPrev = smaShort->line[candleIndex - 1];
        double longPrev = smaLong->line[candleIndex - 1];

        double buyPrice = getBuyPrice();
        bool isStopLoss = inPosition() && (close <= buyPrice * (1.0 - stopLossPct));
        bool isTakeProfit = inPosition() && (close >= buyPrice * (1.0 + takeProfitPct));
        if (isTakeProfit)
            Orderer(candleIndex, false, "Take Profit");
        
        if (isStopLoss)
            Orderer(candleIndex, false, "Stop Loss");

        // Golden Cross: short crosses above long -> BUY
        if (!inPosition() && shortPrev <= longPrev && shortNow > longNow)
            Orderer(candleIndex, true, "Golden Cross BUY");

        // Death Cross: short crosses below long -> SELL
        if (inPosition() && shortPrev >= longPrev && shortNow < longNow)
            Orderer(candleIndex, false, "Death Cross SELL");
    }
};
