#pragma once
#include "Strategy.h"
#include "Indicator.h"

// RSI Strategy: Buy on oversold recovery, sell on overbought exhaustion
// BUY: RSI crosses above oversold level (e.g. 30)
// SELL: RSI crosses below overbought level (e.g. 70)
class RSIStrategy : public Strategy
{
public:
    RSIStrategy(OHLC* data, std::vector<int> params) : Strategy(data, params) {};
    ~RSIStrategy() = default;

    Indicator* rsi = nullptr;
    double stopLossPct = 0.05;
    double takeProfitPct = 0.05;

    void init()
    {
        int period = param(0, 14);
        int oversold = param(1, 30);
        int overbought = param(2, 70);
        stopLossPct = param(3, 5) / 100.0;
        takeProfitPct = param(4, 5) / 100.0;

        rsi = addIndicator<RSI>(m_Data, period);
        m_oversold = oversold;
        m_overbought = overbought;
    }

    void next(int candleIndex)
    {
        if (candleIndex < 1) return;

        double close = m_Data->close[candleIndex];
        double rsiNow = rsi->line[candleIndex];
        double rsiPrev = rsi->line[candleIndex - 1];

        double buyPrice = getBuyPrice();
        bool isStopLoss = inPosition() && (close <= buyPrice * (1.0 - stopLossPct));
        bool isTakeProfit = inPosition() && (close >= buyPrice * (1.0 + takeProfitPct));

        if (isTakeProfit)
            Orderer(candleIndex, false, "Take Profit");
        if (isStopLoss)
            Orderer(candleIndex, false, "Stop Loss");

        // BUY: RSI crosses above oversold (recovery from oversold)
        if (!inPosition() && rsiNow < m_oversold)
            Orderer(candleIndex, true, "RSI Oversold BUY");

        // SELL: RSI crosses below overbought (reversal from overbought)
        if (inPosition() && rsiNow > m_overbought)
            Orderer(candleIndex, false, "RSI Overbought SELL");
    }

private:
    int m_oversold = 30;
    int m_overbought = 70;
};
