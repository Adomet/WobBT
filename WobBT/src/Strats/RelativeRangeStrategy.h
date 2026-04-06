#pragma once
#include "Strategy.h"
#include "Indicator.h"

// Relative Range Strategy: Buy on oversold (bearish exhaustion), sell on overbought
// RR = 100*(close-open)/avg_range; positive = bullish, negative = bearish
// BUY: RR < oversold (e.g. -50) - contrarian entry after strong bearish candle
// SELL: RR > overbought (e.g. 50) or TP/SL
class RelativeRangeStrategy : public Strategy
{
public:
    RelativeRangeStrategy(OHLC* data, std::vector<int> params) : Strategy(data, params) {};
    ~RelativeRangeStrategy() = default;

    Indicator* rr = nullptr;
    double stopLossPct = 0.05;
    double takeProfitPct = 0.05;

    void init()
    {
        int smoothingLength = param(0, 14);
        int rangeLength = param(1, 14);
        m_oversold = -1 * param(2, 50);  // oversold: positive param, negate in strategy
        m_overbought = param(3, 50);
        stopLossPct = param(4, 5) / 100.0;
        takeProfitPct = param(5, 5) / 100.0;

        rr = addIndicator<RelativeRange>(m_Data, smoothingLength, rangeLength);
    }

    void next(int candleIndex)
    {
        if (candleIndex < 1) return;

        double close = m_Data->close[candleIndex];
        double rrNow = rr->line[candleIndex];

        double buyPrice = getBuyPrice();
        bool isStopLoss = inPosition() && (close <= buyPrice * (1.0 - stopLossPct));
        bool isTakeProfit = inPosition() && (close >= buyPrice * (1.0 + takeProfitPct));

        if (isTakeProfit)
            Orderer(candleIndex, false, "Take Profit");
        if (isStopLoss)
            Orderer(candleIndex, false, "Stop Loss");

        // SELL: RR above overbought (strong bullish exhaustion)
        if (inPosition() && rrNow > m_overbought)
            Orderer(candleIndex, false, "RR Overbought SELL");

        // BUY: RR below oversold (contrarian after strong bearish)
        if (!inPosition() && rrNow < m_oversold)
            Orderer(candleIndex, true, "RR Oversold BUY");
    }

private:
    double m_oversold = -50;
    double m_overbought = 50;
};
