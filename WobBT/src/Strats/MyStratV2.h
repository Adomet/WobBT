#pragma once
#include "Strategy.h"
#include "Indicator.h"
#include "Debug.h"

class MyStratV2 : public Strategy
{
public:
    MyStratV2(OHLC* data, std::vector<int> params) :Strategy(data, params) {};
    ~MyStratV2() = default;

public:
    // V5-compatible indicators and state
    Indicator* strend = nullptr;
    Indicator* bull_rsi = nullptr;
    Indicator* bull_tema = nullptr;
    Indicator* bear_rsi = nullptr;
    Indicator* bear_tema = nullptr;
    Indicator* ar = nullptr;
    Indicator* ewo = nullptr;

    bool isbull = false;
    bool isbuyready = false;

    double bull_ewo_offset = 0.0;
    double bull_rsi_high = 0.0;
    double bull_rsi_low = 0.0;
    double bull_takeprofit = 0.0;

    double bear_ewo_offset = 0.0;
    double bear_rsi_high = 0.0;
    double bear_rsi_low = 0.0;
    double bear_takeprofit = 0.0;

    double stop_loss = 0.0;
    double timeProfitRetioDropRate = 0.0;
    double hardSTPDefault = 160 / 1000.0;

    int bull_tema_div_high = 1;
    int bull_tema_div_low = 1;
    int bear_tema_div_high = 1;
    int bear_tema_div_low = 1;

public:
    void init()
    {
        const double stMulti = param(0, 100) / 100.0;
        const int bullTemaPeriod = param(3, 1);
        const int bearRsiPeriod = param(7, 1);
        const int bearTemaPeriod = param(10, 1);
        const int ewoFastPeriod = param(18, 5);
        const int ewoSlowPeriod = param(19, 35);

        strend = addIndicator<SuperTrend>(m_Data, 3, stMulti);
        bull_rsi = addIndicator<RSI>(m_Data, 3);
        bull_tema = addIndicator<TEMA>(m_Data, bullTemaPeriod);
        bear_rsi = addIndicator<RSI>(m_Data, bearRsiPeriod);
        bear_tema = addIndicator<TEMA>(m_Data, bearTemaPeriod);
        ar = addIndicator<AverageRage>(m_Data, 130);
        ewo = addIndicator<EWO>(m_Data, ewoFastPeriod, ewoSlowPeriod);

        bull_ewo_offset = param(16, 0) / 1000.0;
        bull_rsi_high = param(1, 0) / 10.0;
        bull_rsi_low = param(2, 0) / 10.0;
        bull_takeprofit = param(6, 0) / 10000.0;

        bear_ewo_offset = param(17, 0) / 1000.0;
        bear_rsi_high = param(8, 0) / 10.0;
        bear_rsi_low = param(9, 0) / 10.0;
        bear_takeprofit = param(13, 0) / 10000.0;

        stop_loss = param(14, 0) / 10000.0;
        timeProfitRetioDropRate = param(15, 0) / 100000.0;

        bull_tema_div_high = param(4, 1);
        bull_tema_div_low = param(5, 1);
        bear_tema_div_high = param(11, 1);
        bear_tema_div_low = param(12, 1);
    }
    void next(int candleIndex)
    {
        const double close = m_Data->close[candleIndex];

        const double arLine = ar->line[candleIndex];
        const double ewoLine = ewo->line[candleIndex];

        const double strd = strend->line[candleIndex];
        const double bullRsi = bull_rsi->line[candleIndex];
        const double bullTema = bull_tema->line[candleIndex];
        const double bearRsi = bear_rsi->line[candleIndex];
        const double bearTema = bear_tema->line[candleIndex];

        const double bull_diff_tema_heigh = bullTema + ((bullTema / bull_tema_div_high) * 10.0);
        const double bull_diff_tema_low = bullTema - ((bullTema / bull_tema_div_low) * 10.0);
        const double bear_diff_tema_heigh = bearTema + ((bearTema / bear_tema_div_high) * arLine * 3.7);
        const double bear_diff_tema_low = bearTema - ((bearTema / bear_tema_div_low) * 10.0);

        const double buyPrice = getBuyPrice();
        const bool isStop = inPosition() && (close <= buyPrice - (buyPrice * stop_loss * arLine));
        isbull = (strd < close);

        if (isbull)
        {
            const bool bull_rsibuytrigger = bullRsi <= bull_rsi_low;
            const bool bull_rsiselltrigger = bullRsi >= bull_rsi_high;
            const bool bull_avgdiffselltrigger = close >= bull_diff_tema_heigh;
            const bool bull_avgdiffbuytrigger = close <= bull_diff_tema_low;
            const bool bull_isTakeProfit = inPosition() && (close >= buyPrice + (buyPrice * bull_takeprofit));
            const bool bull_ewo_trigger = ewoLine <= -bull_ewo_offset;

            if (bull_avgdiffbuytrigger && (bull_rsibuytrigger || bull_ewo_trigger))
                isbuyready = true;
            if (bull_rsiselltrigger && bull_avgdiffselltrigger)
                Orderer(candleIndex, false, "Bull_IND SELL");
            if (bull_isTakeProfit)
                Orderer(candleIndex, false, "Bull_TAKE PROFIT");
            if (isStop)
                Orderer(candleIndex, false, "Bull_STOPPED");
        }
        else
        {
            const bool bear_rsibuytrigger = bearRsi <= bear_rsi_low;
            const bool bear_rsiselltrigger = bearRsi >= bear_rsi_high;
            const bool bear_avgdiffselltrigger = close >= bear_diff_tema_heigh;
            const bool bear_avgdiffbuytrigger = close <= bear_diff_tema_low;
            const bool bear_isTakeProfit = inPosition() && (close >= buyPrice + (buyPrice * bear_takeprofit * arLine));
            const bool bear_ewo_trigger = ewoLine <= -bear_ewo_offset;

            if (bear_avgdiffbuytrigger && bear_rsibuytrigger && bear_ewo_trigger)
                isbuyready = true;
            if (bear_rsiselltrigger && bear_avgdiffselltrigger)
                Orderer(candleIndex, false, "Bear_IND SELL");
            if (bear_isTakeProfit)
                Orderer(candleIndex, false, "Bear_TAKE PROFIT");
            if (isStop)
                Orderer(candleIndex, false, "Bear_STOPPED");
        }

        if (isbuyready)
        {
            isbuyready = false;
            Orderer(candleIndex, true, "CANDLE_TRIG BUY");
        }

        const bool TimeProfitRatioSTP = inPosition() && !isbull && ((close - buyPrice) / buyPrice >= (bull_takeprofit - (timeProfitRetioDropRate * getPosCandleCount())));
        const bool hardSTP = inPosition() && (close <= buyPrice - (buyPrice * hardSTPDefault));

        if (TimeProfitRatioSTP)
            Orderer(candleIndex, false, "Time_Profit SELL");

        if (hardSTP)
            Orderer(candleIndex, false, "HARD_STP SELL");
    }
};