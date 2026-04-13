#pragma once
#include <cmath>
#include "Strategy.h"
#include "Indicator.h"

class MyStratV4 : public Strategy
{
public:
	MyStratV4(OHLC* data, std::vector<int> params) : Strategy(data, params) {};
	~MyStratV4() = default;


	Indicator* rsi = nullptr;
	Indicator* tema = nullptr;

	double rsi_trigger_high = 75;
	double rsi_trigger_low = 25;

	double rsi_high = 75;
	double rsi_low = 25;

	double tema_high = 30;
	double tema_low = 30;

	double takeprofit = 0.05;
	double takeprofitStep = 0.05;
	double effectiveTakeProfit = 0.0;
	double stopLossPct = 0.03;
	double timeTakeProfitDrop = 0.0003;

	bool buyTrigger = false;
	bool sellTrigger = false;

	int RuinAfterCandle = 0;

	void init()
	{
		rsi = addIndicator<RSI>(m_Data, param(0, 21));
		rsi_trigger_high = param(1, 75);
		rsi_trigger_low = param(2, 25);

		rsi_high = param(3, 75);
		rsi_low = param(4, 25);

		tema = addIndicator<TEMA>(m_Data, param(5, 51));
		tema_high = param(6, 30);
		tema_low = param(7, 30);

		takeprofit = param(8, 50) / 1000.0;
		stopLossPct = param(9, 50) / 1000.0;

		takeprofitStep = param(10, 5) / 10000.0;
		RuinAfterCandle = param(11, 50);

		timeTakeProfitDrop = param(12,5) / 1000000.0;
	}

	void next(int candleIndex)
	{
		if (candleIndex < 1) return;

		double close = m_Data->close[candleIndex];
		double open = m_Data->open[candleIndex];

		double buyPrice = getBuyPrice();
		bool inPos = inPosition();

		double rsiNow = rsi->line[candleIndex];
		double rsiTrigger = rsi->line[candleIndex];

		double temaNow = tema->line[candleIndex];

		double diff_tema_heigh = temaNow + (temaNow * tema_high / 10000.0);
		double diff_tema_low = temaNow - (temaNow * tema_low / 10000.0);

		double effectiveSTP = m_posCandleCount > RuinAfterCandle ? -2 / 1000.0 : stopLossPct;


		buyTrigger |= rsiTrigger < rsi_trigger_low && close <= diff_tema_low;
		sellTrigger |= rsiTrigger > rsi_trigger_high && close >= diff_tema_heigh;



		if (inPos)
		{
			int candlesToCap = (int)(takeprofit / takeprofitStep);
			int remain = m_posCandleCount - candlesToCap;

			if (remain < 0)
				effectiveTakeProfit = takeprofitStep * m_posCandleCount;
			else
				effectiveTakeProfit = takeprofit - (timeTakeProfitDrop * remain);
		}

		if (buyTrigger && rsiNow > rsi_low && !inPos)
		{
			Orderer(candleIndex, true, "BUY");
			cleanTriggers();
		}

		if (sellTrigger && rsiNow < rsi_high && inPos)
		{
			Orderer(candleIndex, false, "SELL");
			cleanTriggers();
		}


		bool isTakeProfit = (close >= buyPrice * (1.0 + effectiveTakeProfit));
		if (isTakeProfit && inPos)
		{
			Orderer(candleIndex, false, "TAKE PROFIT");
			cleanTriggers();
		}


		bool isStopLoss = (close <= buyPrice * (1.0 - effectiveSTP));
		if (isStopLoss && inPos)
		{
			Orderer(candleIndex, false, "STOPPED");
			cleanTriggers();
		}
	}

	void cleanTriggers()
	{
		sellTrigger = buyTrigger = false;
	}
};
