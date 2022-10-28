#pragma once
#include "Strategy.h"
#include "Indicator.h"
#include "Debug.h"

class MyStratV1 : public Strategy
{
    //Ind List: SuperTrend,CrossOver,TD9,ADX,ATR,RSI,TEMA,EMA,SMA
public:
    MyStratV1(OHLC* data, std::vector<int> params) :Strategy(data, params) {};
    ~MyStratV1() = default;

public:
    // Strat Indicator and parameters
    Indicator* strend    = new SuperTrend(m_Data, m_params[0], m_params[1] / 100.0);
    Indicator* td9       = new TD9(m_Data);
    Indicator* adx       = new ADX(m_Data, 13);
    Indicator* atr       = new ATR(m_Data, 9);
    bool isbull = false;

    //BULL
    Indicator* bull_rsi  = new RSI(m_Data, m_params[2]);
    double bull_rsi_high = m_params[3] / 10.0;
    double bull_rsi_low  = m_params[4] / 10.0;
    Indicator* bull_tema = new TEMA(m_Data, m_params[5]);
    //double bull_diff_ema_heigh = bull_tema + (bull_tema / m_params[6] * 10);
    //double bull_diff_ema_low   = bull_tema - (bull_tema / m_params[7] * 10);
    double bull_takeprofit = m_params[8] / 10000.0;

    //BEAR
    Indicator* bear_rsi  = new RSI(m_Data, m_params[9]);
    double bear_rsi_high = m_params[10] / 10.0;
    double bear_rsi_low  = m_params[11] / 10.0;
    Indicator* bear_tema = new TEMA(m_Data, m_params[12]);
    //double bear_diff_ema_heigh = bear_tema + (bear_tema / m_params[13] * 10);
    //double bear_diff_ema_low   = bear_tema - (bear_tema / m_params[14] * 10);
    double bear_takeprofit = m_params[15] / 10000.0;

    //GENERAL
    double stop_loss = m_params[16] / 10000.0;
    double timeProfitRetioDropRate = m_params[17] / 1000000.0;
    double hardSTPDefault = m_params[18] / 1000.0;

    //STUFF
    int    posCandleCount = 0;
    bool   isbuyready = false;

public:
    void init()
    {
        // Create Indicators add them to Indicators list
        m_Inds.push_back(strend);
        m_Inds.push_back(td9);
        m_Inds.push_back(adx);
        m_Inds.push_back(atr);
        m_Inds.push_back(bull_rsi);
        m_Inds.push_back(bull_tema);
        m_Inds.push_back(bear_rsi);
        m_Inds.push_back(bear_tema);
        
        //Get most await period of indicators
        for each (auto var in m_Inds)
        {
            auto tmp = var->init_period;
            if (m_init_all_ind_periods < tmp)
                m_init_all_ind_periods = tmp;
        }

    }
    void next(int candleIndex)
    {
        //Get data of candle and indicators 
        //check if all indicator inits done
        if (m_init_all_ind_periods > candleIndex)
            return;


        // get indicators
        double strd      = m_Inds[0]->line[candleIndex];
        double td9       = m_Inds[1]->line[candleIndex];
        double adx       = m_Inds[2]->line[candleIndex];
        double atr       = m_Inds[3]->line[candleIndex];
        double bull_rsi  = m_Inds[4]->line[candleIndex];
        double bull_tema = m_Inds[5]->line[candleIndex];
        double bear_rsi  = m_Inds[6]->line[candleIndex];
        double bear_tema = m_Inds[7]->line[candleIndex];

        //==============================================================================================================================
        //Init addictional lines
        double bull_diff_tema_heigh = bull_tema + (bull_tema / m_params[6]  * 10.0);
        double bull_diff_tema_low   = bull_tema - (bull_tema / m_params[7]  * 10.0);
        double bear_diff_tema_heigh = bear_tema + (bear_tema / m_params[13] * 10.0);
        double bear_diff_tema_low   = bear_tema - (bear_tema / m_params[14] * 10.0);

        bool adxtrigger           = adx >= 26.0;
        bool td9selltrigger       = td9 >= 10.0;
        bool isStop               = m_Data->close[candleIndex] <= m_buyprice - (m_buyprice * stop_loss) - (atr * 81.0 / 1000.0);
        bool candleDiffbuytrigger = (58.0 / 1000.0) >= (1.0 - (m_Data->close[candleIndex] / m_Data->open[candleIndex]));

        isbull = (strd < m_Data->close[candleIndex]);

        //Trade Logic
        if (isbull)
        {
            bool bull_rsiselltrigger     = bull_rsi >= bull_rsi_high;
            bool bull_rsibuytrigger      = bull_rsi <= bull_rsi_low;
            bool bull_avgdiffselltrigger = m_Data->close[candleIndex] >= bull_diff_tema_heigh;
            bool bull_avgdiffbuytrigger  = m_Data->close[candleIndex]  <= bull_diff_tema_low;
            bool bull_isTakeProfit       = m_Data->close[candleIndex] >= m_buyprice + (m_buyprice * bull_takeprofit) && (m_buyprice != -1);

            if (bull_rsibuytrigger && bull_avgdiffbuytrigger)
                isbuyready = true;
            else if (bull_rsiselltrigger && bull_avgdiffselltrigger && td9selltrigger)
                Orderer(candleIndex, false, "Bull_IND SELL");
            else if (bull_isTakeProfit)
                Orderer(candleIndex, false, "Bull_TAKE PROFIT");
            else if (isStop && adxtrigger)
                Orderer(candleIndex, false, "Bull_STOPPED");
        }
        else
        {
            bool bear_rsiselltrigger     = bear_rsi >= bear_rsi_high;
            bool bear_rsibuytrigger      = bear_rsi <= bear_rsi_low;
            bool bear_avgdiffselltrigger = m_Data->close[candleIndex] >= bear_diff_tema_heigh;
            bool bear_avgdiffbuytrigger  = m_Data->close[candleIndex]  <= bear_diff_tema_low;
            bool bear_isTakeProfit       = m_Data->close[candleIndex] >= m_buyprice + (m_buyprice * bear_takeprofit) && (m_buyprice != -1);

            if (bear_rsibuytrigger && bear_avgdiffbuytrigger)
                isbuyready = true;
            else if (bear_rsiselltrigger && bear_avgdiffselltrigger)
                Orderer(candleIndex, false, "Bear_IND SELL");
            else if (bear_isTakeProfit)
                Orderer(candleIndex, false, "Bear_TAKE PROFIT");
            else if (isStop && adxtrigger)
                Orderer(candleIndex, false, "Bear_STOPPED");
        }

        if (candleDiffbuytrigger && isbuyready)
        {
            isbuyready = false;
            double closeval = m_Data->close[candleIndex];
            Orderer(candleIndex, true, "CANDLE_TRIG BUY");
        }

        if (m_buyprice != -1)
            posCandleCount += 1;
        else
            posCandleCount = 0;

        //### NEW STUFF ###
        bool TimeProfitRatioSTP = (m_Data->close[candleIndex] - m_buyprice) / m_buyprice >= ((bull_takeprofit)-(timeProfitRetioDropRate * (posCandleCount))) && !isbull;
        bool hardSTP            = m_Data->close[candleIndex] <= m_buyprice - (m_buyprice * hardSTPDefault ) && !isbull;

        if (TimeProfitRatioSTP)
            Orderer(candleIndex, false, "Time_Profit SELL");

        if (hardSTP && adxtrigger)
            Orderer(candleIndex, false, "HARD_STP SELL");



        //Print Last Candle
        if ((false || candleIndex == m_Data->close.size() - 1) && false)
        {
            Debug::Log("------------------------------------------------------------------------------------------");
            Debug::Log("candleIndex:"      + std::to_string(candleIndex) + " => open:" + std::to_string(m_Data->open[candleIndex]) + " high:" + std::to_string(m_Data->high[candleIndex]) + " low:" + std::to_string(m_Data->low[candleIndex]) + " close:" + std::to_string(m_Data->close[candleIndex]) + " volume:" + std::to_string(m_Data->volume[candleIndex]));
            Debug::Log("isbull: "          + std::to_string(isbull));
            Debug::Log("diff_tema_heigh: " + std::to_string(bull_diff_tema_heigh));
            Debug::Log("diff_tema_low: "   + std::to_string(bear_diff_tema_low));
            Debug::Log("trade_count: "     + std::to_string(m_tradeCount));
        }

    }

};