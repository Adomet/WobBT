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

    void init()
    {
        // Create Indicators add them to Indicators list
        // Indicator List: SMA,EMA,DEMA,TEMA,RSI,ATR,ADX,SuperTrend,TD9,CrossOver
        // 

        m_Inds.push_back(new SMA(m_Data,        m_params[0]));
        m_Inds.push_back(new EMA(m_Data,        m_params[1]));
        m_Inds.push_back(new DEMA(m_Data,       m_params[2]));
        m_Inds.push_back(new TEMA(m_Data,       m_params[3]));
        m_Inds.push_back(new RSI(m_Data,        m_params[4]));
        m_Inds.push_back(new ATR(m_Data,        m_params[5]));
        m_Inds.push_back(new ADX(m_Data,        m_params[6]));
        m_Inds.push_back(new SuperTrend(m_Data, m_params[7]));
        m_Inds.push_back(new TD9(m_Data));

        auto ema  = m_Inds[1];
        auto dema = m_Inds[2];
        m_Inds.push_back(new CrossOver(m_Data, dema->line, ema->line));


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
        // check if all indicators are ready
        if (m_init_all_ind_periods > candleIndex)
            return;

        //get candle values
        float open   = m_Data->open[candleIndex];
        float high   = m_Data->high[candleIndex];
        float low    = m_Data->low[candleIndex];
        float close  = m_Data->close[candleIndex];
        float volume = m_Data->volume[candleIndex];

        // get indicators
        double sma   = m_Inds[0]->line[candleIndex];
        double ema   = m_Inds[1]->line[candleIndex];
        double dema  = m_Inds[2]->line[candleIndex];
        double tema  = m_Inds[3]->line[candleIndex];
        double rsi   = m_Inds[4]->line[candleIndex];
        double atr   = m_Inds[5]->line[candleIndex];
        double adx   = m_Inds[6]->line[candleIndex];
        double strd  = m_Inds[7]->line[candleIndex];
        double td9   = m_Inds[8]->line[candleIndex];
        double crss  = m_Inds[9]->line[candleIndex];

        //Print Last Candle
        if (candleIndex == m_Data->close.size()-1)
        {
            Debug::Log("------------------------------------------------------------------------------------------");
            Debug::Log("candleIndex:" + std::to_string(candleIndex)+" => open:" + std::to_string(open) + " high:" + std::to_string(high) + " low:" + std::to_string(low) + " close:" + std::to_string(close) + " volume:" + std::to_string(volume));
            Debug::Log("sma:  " + std::to_string(sma));
            Debug::Log("ema:  " + std::to_string(ema));
            Debug::Log("dema: " + std::to_string(dema));
            Debug::Log("tema: " + std::to_string(tema));
            Debug::Log("rsi:  " + std::to_string(rsi));
            Debug::Log("atr:  " + std::to_string(atr));
            Debug::Log("adx:  " + std::to_string(adx));
            Debug::Log("strd: " + std::to_string(strd));
            Debug::Log("td9:  " + std::to_string(td9));
            Debug::Log("crss: " + std::to_string(crss));
        }


        //==============================================================================================================================
        //Trade Logic
        if (rsi > 70)
            Orderer(candleIndex, true);

        if (rsi < 30)
            Orderer(candleIndex, false);

    }

};