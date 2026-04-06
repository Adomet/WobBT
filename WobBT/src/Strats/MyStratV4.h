#pragma once
#include "Strategy.h"
#include "Indicator.h"

class MyStratV4 : public Strategy
{
public:
	MyStratV4(OHLC* data, std::vector<int> params) : Strategy(data, params) {};
	~MyStratV4() = default;



	Indicator* rsi = nullptr;
	Indicator* tema = nullptr;
	Greed* greedInd = nullptr;

	double rsi_high = 75;
	double rsi_low = 25;
	double tema_high = 30;
	double tema_low = 30;

	double takeprofit = 0.05;
	double stopLossPct = 0.03;

	void init()
	{
		rsi = addIndicator<RSI>(m_Data, param(0, 21));
		rsi_high = param(1, 75);
		rsi_low = param(2, 25);



		tema = addIndicator<EMA>(m_Data, param(3, 51));
		tema_high = param(4, 30);
		tema_low = param(5, 30);

		takeprofit = param(6, 50) / 1000.0;
		stopLossPct = param(7, 50) / 1000.0;

		greedInd = addIndicator<Greed>(m_Data);
	}

	void next(int candleIndex)
	{
		if (candleIndex < 1) return;

		greedInd->update(candleIndex, m_trades);
		double greed = greedInd->line[candleIndex];

		//Debug::Log("Greed: " + std::to_string(greed));
		double close = m_Data->close[candleIndex];
		double buyPrice = getBuyPrice();

		double rsiNow = rsi->line[candleIndex];
		double temaNow = tema->line[candleIndex];

		double diff_tema_heigh = temaNow + (temaNow * tema_high / 1000);
		double diff_tema_low = temaNow - (temaNow * tema_low / 1000);


		bool rsiBuy = rsiNow <= rsi_low;
		bool rsiSell = rsiNow >= rsi_high;

		bool temaBuy = close <= diff_tema_low;
		bool temaSell = close >= diff_tema_heigh;

		if (temaBuy && rsiBuy)
			Orderer(candleIndex, true, "BUY");

		if (temaSell && rsiSell)
			Orderer(candleIndex, false, "SELL");


		double tpEff = takeprofit * greed;
		double slEff = stopLossPct * greed;

		bool isTakeProfit = (close >= buyPrice * (1.0 + tpEff));
		if (isTakeProfit)
		{
			Orderer(candleIndex, false, "TAKE PROFIT");
		}

		bool isStopLoss = (close <= buyPrice * (1.0 - slEff));
		if (isStopLoss)
		{
			Orderer(candleIndex, false, "STOPPED");
		}
	}
};
