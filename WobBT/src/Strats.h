#pragma once
#include <algorithm>
#include "Strategy.h"
#include "Indicator.h"
#include "Debug.h"

class MyStratV1 : public Strategy
{
public:
    MyStratV1(OHLC* data, std::vector<int> params) :Strategy(data, params) {};
    ~MyStratV1() = default;

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
    int posCandleCount = 0;

    // Cached params (aligned to MyStratV5.py)
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

    int p(int idx, int fallback) const
    {
        if (idx < 0 || idx >= (int)m_params.size())
            return fallback;
        return m_params[idx];
    }

    static int nz(int value, int fallback = 1)
    {
        return value == 0 ? fallback : value;
    }

public:
    void init()
    {
        const double stMulti = std::max(1.0, p(0, 100) / 100.0);
        const int bullTemaPeriod = std::max(1, p(3, 1));
        const int bearRsiPeriod = std::max(1, p(7, 1));
        const int bearTemaPeriod = std::max(1, p(10, 1));
        const int ewoFastPeriod = std::max(1, p(18, 5));
        const int ewoSlowPeriod = std::max(1, p(19, 35));

        strend = new SuperTrend(m_Data, 3, stMulti);
        bull_rsi = new RSI(m_Data, 3);
        bull_tema = new TEMA(m_Data, bullTemaPeriod);
        bear_rsi = new RSI(m_Data, bearRsiPeriod);
        bear_tema = new TEMA(m_Data, bearTemaPeriod);
        ar = new AverageRage(m_Data, 130);
        ewo = new EWO(m_Data, ewoFastPeriod, ewoSlowPeriod);

        bull_ewo_offset = p(16, 0) / 1000.0;
        bull_rsi_high = p(1, 0) / 10.0;
        bull_rsi_low = p(2, 0) / 10.0;
        bull_takeprofit = p(6, 0) / 10000.0;

        bear_ewo_offset = p(17, 0) / 1000.0;
        bear_rsi_high = p(8, 0) / 10.0;
        bear_rsi_low = p(9, 0) / 10.0;
        bear_takeprofit = p(13, 0) / 10000.0;

        stop_loss = p(14, 0) / 10000.0;
        timeProfitRetioDropRate = p(15, 0) / 1000000.0;

        bull_tema_div_high = nz(p(4, 1), 1);
        bull_tema_div_low = nz(p(5, 1), 1);
        bear_tema_div_high = nz(p(11, 1), 1);
        bear_tema_div_low = nz(p(12, 1), 1);

        m_Inds.push_back(strend);
        m_Inds.push_back(bull_rsi);
        m_Inds.push_back(bull_tema);
        m_Inds.push_back(bear_rsi);
        m_Inds.push_back(bear_tema);
        m_Inds.push_back(ar);
        m_Inds.push_back(ewo);

        // Get most warmup period of indicators
        for (auto* var : m_Inds)
        {
            auto tmp = var->init_period;
            if (m_init_all_ind_periods < tmp)
                m_init_all_ind_periods = tmp;
        }
    }
    void next(int candleIndex)
    {
        // Check if all indicators are initialized.
        if (m_init_all_ind_periods > candleIndex)
            return;

        const double close = m_Data->close[candleIndex];
        const bool inPos = m_buyprice != -1;

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

        const bool isStop = inPos && (close <= m_buyprice - (m_buyprice * stop_loss * arLine));
        isbull = (strd < close);

        if (isbull)
        {
            const bool bull_rsibuytrigger = bullRsi <= bull_rsi_low;
            const bool bull_rsiselltrigger = bullRsi >= bull_rsi_high;
            const bool bull_avgdiffselltrigger = close >= bull_diff_tema_heigh;
            const bool bull_avgdiffbuytrigger = close <= bull_diff_tema_low;
            const bool bull_isTakeProfit = inPos && (close >= m_buyprice + (m_buyprice * bull_takeprofit));
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
            const bool bear_isTakeProfit = inPos && (close >= m_buyprice + (m_buyprice * bear_takeprofit * arLine));
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

        if (m_buyprice != -1)
            posCandleCount += 1;
        else
            posCandleCount = 0;

        const bool TimeProfitRatioSTP = inPos && !isbull && ((close - m_buyprice) / m_buyprice >= (bull_takeprofit - (timeProfitRetioDropRate * posCandleCount)));
        const bool hardSTP = inPos && (close <= m_buyprice - (m_buyprice * hardSTPDefault));

        if (TimeProfitRatioSTP)
            Orderer(candleIndex, false, "Time_Profit SELL");

        if (hardSTP)
            Orderer(candleIndex, false, "HARD_STP SELL");
    }

    void Plot(bool plotIndicators = true)
    {
        this->Strategy::Plot(&m_equityCurve, plotIndicators);
    }

};