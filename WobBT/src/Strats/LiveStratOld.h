#pragma once
#include "Strategy.h"
#include "Indicator.h"

class LiveStratOld : public Strategy
{
public:
    LiveStratOld(OHLC* data, std::vector<int> params) : Strategy(data, params) {};
    ~LiveStratOld() = default;

    Indicator* strend = nullptr;
    Indicator* td9Ind = nullptr;
    Indicator* adx = nullptr;
    Indicator* ar = nullptr;
    Indicator* bull_rsi = nullptr;
    Indicator* bull_tema = nullptr;
    Indicator* bear_rsi = nullptr;
    Indicator* bear_tema = nullptr;

    bool isbull = false;
    bool isbuyready = false;

    double bull_rsi_high = 0;
    double bull_rsi_low = 0;
    double bull_takeprofit = 0;
    int bull_tema_div_high = 1;
    int bull_tema_div_low = 1;

    double bear_rsi_high = 0;
    double bear_rsi_low = 0;
    double bear_takeprofit = 0;
    int bear_tema_div_high = 1;
    int bear_tema_div_low = 1;

    double stop_loss = 0;
    double timeProfitRetioDropRate = 0;
    double hardSTPDefault = 160.0 / 1000.0;
    double bull_tp_per = 16.0 / 1000.0;

    // p0:  SuperTrend period
    // p1:  SuperTrend multiplier (/100, min 1.0)
    // p2:  Bull RSI period
    // p3:  Bull RSI high (/10)
    // p4:  Bull RSI low  (/10)
    // p5:  Bull TEMA period
    // p6:  Bull TEMA div high
    // p7:  Bull TEMA div low
    // p8:  Bull take profit (/10000)
    // p9:  Bear RSI period
    // p10: Bear RSI high (/10)
    // p11: Bear RSI low  (/10)
    // p12: Bear TEMA period
    // p13: Bear TEMA div high
    // p14: Bear TEMA div low
    // p15: Bear take profit (/10000)
    // p16: Stop loss (/10000)
    // p17: timeProfitRetioDropRate (/1000000)
    // p18: AverageRage period

    void init()
    {
        int stPeriod = param(0, 3);
        double stMulti = std::max(param(1, 238) / 100.0, 1.0);
        int bullRsiPeriod = param(2, 3);
        int bullTemaPeriod = param(5, 74);
        int bearRsiPeriod = param(9, 25);
        int bearTemaPeriod = param(12, 119);
        int arPeriod = param(18, 81);

        strend    = addIndicator<SuperTrend>(m_Data, stPeriod, stMulti);
        td9Ind    = addIndicator<TD9>(m_Data);
        adx       = addIndicator<ADX>(m_Data, 13);
        ar        = addIndicator<AverageRage>(m_Data, arPeriod);
        bull_rsi  = addIndicator<RSI>(m_Data, bullRsiPeriod);
        bull_tema = addIndicator<TEMA>(m_Data, bullTemaPeriod);
        bear_rsi  = addIndicator<RSI>(m_Data, bearRsiPeriod);
        bear_tema = addIndicator<TEMA>(m_Data, bearTemaPeriod);

        bull_rsi_high      = param(3, 848) / 10.0;
        bull_rsi_low       = param(4, 161) / 10.0;
        bull_tema_div_high = param(6, 186);
        bull_tema_div_low  = param(7, 402);
        bull_takeprofit    = param(8, 1626) / 10000.0;

        bear_rsi_high      = param(10, 517) / 10.0;
        bear_rsi_low       = param(11, 350) / 10.0;
        bear_tema_div_high = param(13, 165);
        bear_tema_div_low  = param(14, 340);
        bear_takeprofit    = param(15, 1093) / 10000.0;

        stop_loss              = param(16, 585) / 10000.0;
        timeProfitRetioDropRate = param(17, 261) / 1000000.0;
    }

    void next(int candleIndex)
    {
        const double close = m_Data->close[candleIndex];
        const double open  = m_Data->open[candleIndex];

        const double strd    = strend->line[candleIndex];
        const double td9     = td9Ind->line[candleIndex];
        const double adxLine = adx->line[candleIndex];
        const double arLine  = ar->line[candleIndex];
        const double bullRsi  = bull_rsi->line[candleIndex];
        const double bullTema = bull_tema->line[candleIndex];
        const double bearRsi  = bear_rsi->line[candleIndex];
        const double bearTema = bear_tema->line[candleIndex];

        const double buyPrice = getBuyPrice();
        const bool adxTrigger        = adxLine >= 26.0;
        const bool td9SellTrigger    = td9 >= 10.0;
        const bool isStop            = inPosition() && (close <= buyPrice - (buyPrice * stop_loss));
        const bool isProfit          = inPosition() && (close > buyPrice);
        const bool candleDiffBuyTrig = open > 0 && (0.0358 >= (1.0 - (close / open)));
        const bool isSellPer         = inPosition() && (close > buyPrice + (buyPrice * bull_tp_per));

        isbull = (strd < close);

        if (isbull)
        {
            const double bull_diff_tema_high = bullTema + (bullTema / bull_tema_div_high * 10.0);
            const double bull_diff_tema_low  = bullTema - (bullTema / bull_tema_div_low  * 10.0);

            const bool rsibuytrig      = bullRsi <= bull_rsi_low;
            const bool rsiselltrig     = bullRsi >= bull_rsi_high;
            const bool avgdiffselltrig = close >= bull_diff_tema_high;
            const bool avgdiffbuytrig  = close <= bull_diff_tema_low;
            const bool isTakeProfit    = inPosition() && (close >= buyPrice + (buyPrice * bull_takeprofit));

            if (rsibuytrig && avgdiffbuytrig)
                isbuyready = true;
            else if (rsiselltrig && avgdiffselltrig && td9SellTrigger && isSellPer)
                Orderer(candleIndex, false, "Bull_IND SELL");
            else if (isTakeProfit)
                Orderer(candleIndex, false, "Bull_TAKE PROFIT");
            else if (isStop && adxTrigger)
                Orderer(candleIndex, false, "Bull_STOPPED");
        }
        else
        {
            const double bear_diff_tema_high = bearTema + ((bearTema / bear_tema_div_high) * arLine * 3.7);
            const double bear_diff_tema_low  = bearTema - ((bearTema / bear_tema_div_low)  * 10.0);

            const bool rsibuytrig      = bearRsi <= bear_rsi_low;
            const bool rsiselltrig     = bearRsi >= bear_rsi_high;
            const bool avgdiffselltrig = close >= bear_diff_tema_high;
            const bool avgdiffbuytrig  = close <= bear_diff_tema_low;
            const bool isTakeProfit    = inPosition() && (close >= buyPrice + (buyPrice * bear_takeprofit));

            if (rsibuytrig && avgdiffbuytrig)
                isbuyready = true;
            else if (rsiselltrig && avgdiffselltrig && isSellPer)
                Orderer(candleIndex, false, "Bear_IND SELL");
            else if (isTakeProfit)
                Orderer(candleIndex, false, "Bear_TAKE PROFIT");
            else if (isStop && adxTrigger)
                Orderer(candleIndex, false, "Bear_STOPPED");
        }

        if (candleDiffBuyTrig && isbuyready)
        {
            isbuyready = false;
            Orderer(candleIndex, true, "BUYED");
        }

        const bool timeProfitSTP = inPosition() && !isbull && ((close - buyPrice) / buyPrice >= (bull_takeprofit - (timeProfitRetioDropRate * getPosCandleCount())));
        const bool hardSTP = inPosition() && !isbull && (close <= buyPrice - (buyPrice * hardSTPDefault));

        if (timeProfitSTP && isProfit)
            Orderer(candleIndex, false, "Time/Profit SELL");

        if (hardSTP && adxTrigger)
            Orderer(candleIndex, false, "HARD_STP SELL");
    }
};
