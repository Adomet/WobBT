#pragma once
#include "OHLC.h"
#include <vector>
#include <iostream>
#include <deque>
#include <cmath>

//Indicator List: SMA,EMA,DEMA,TEMA,RSI,ATR,ADX,AverageRage,NormalizedRange,
//                AverageDiff,ATD,RelativeRange,EWO,TD9,CrossOver,SuperTrend

class Indicator
{
public:
	Indicator(OHLC* ohlc) :m_ohlc(ohlc) {};
	~Indicator() = default;
	virtual void init() {};

public:
	std::string m_name = "Indicator";
	std::vector<double> line;
	OHLC* m_ohlc;
	int init_period = 0;
	bool isSubplot = false;
};

class SMA : public Indicator
{
public:
	SMA(OHLC* ohlc, int period) :Indicator(ohlc) { m_period = std::max(min_period, period); init(); m_name = "SMA"; };
	~SMA() {};
	void init()
	{
		line.resize(m_ohlc->close.size(), 0.0);
		for (size_t i = m_period - 1; i < m_ohlc->close.size(); i++)
		{
			double sum = 0.0;
			for (int j = 0; j < m_period; j++)
			{
				sum += m_ohlc->close[i - j];
			}
			line[i] = sum / m_period;
		}
		init_period = m_period - 1;
	}
public:
	int min_period = 1;
	int m_period;
};

class EMA : public Indicator
{
public:
	EMA(OHLC* ohlc, int period) :Indicator(ohlc) { m_period = std::max(min_period, period); init(); m_name = "EMA"; };
	~EMA() {};
	void init()
	{
		line.resize(m_ohlc->close.size(), 0.0);
		double multiplier = 2.0 / (m_period + 1.0);
		
		// First EMA is SMA
		double sum = 0.0;
		for (int i = 0; i < m_period; i++)
		{
			sum += m_ohlc->close[i];
		}
		line[m_period - 1] = sum / m_period;

		// Calculate EMA
		for (size_t i = m_period; i < m_ohlc->close.size(); i++)
		{
			line[i] = (m_ohlc->close[i] - line[i - 1]) * multiplier + line[i - 1];
		}
		init_period = m_period - 1;
	}
public:
	int min_period = 1;
	int m_period;
};

class DEMA : public Indicator
{
public:
	DEMA(OHLC* ohlc, int period) :Indicator(ohlc) { m_period = std::max(min_period, period); init(); m_name = "DEMA"; };
	~DEMA() {};
	void init()
	{
		line.resize(m_ohlc->close.size(), 0.0);
		
		// Calculate first EMA
		std::vector<double> ema1(m_ohlc->close.size(), 0.0);
		double multiplier = 2.0 / (m_period + 1.0);
		
		// First EMA is SMA
		double sum = 0.0;
		for (int i = 0; i < m_period; i++)
		{
			sum += m_ohlc->close[i];
		}
		ema1[m_period - 1] = sum / m_period;

		// Calculate first EMA
		for (size_t i = m_period; i < m_ohlc->close.size(); i++)
		{
			ema1[i] = (m_ohlc->close[i] - ema1[i - 1]) * multiplier + ema1[i - 1];
		}

		// Calculate second EMA of the first EMA
		sum = 0.0;
		for (int i = 0; i < m_period; i++)
		{
			sum += ema1[i];
		}
		line[m_period - 1] = sum / m_period;

		// Calculate second EMA (EMA of EMA1)
		for (size_t i = m_period; i < m_ohlc->close.size(); i++)
		{
			line[i] = (ema1[i] - line[i - 1]) * multiplier + line[i - 1];
		}

		// DEMA = 2*EMA1 - EMA2
		for (size_t i = m_period - 1; i < m_ohlc->close.size(); i++)
		{
			line[i] = 2.0 * ema1[i] - line[i];
		}

		init_period = m_period - 1;
	}
public:
	int min_period = 1;
	int m_period;
};


class TEMA : public Indicator
{
public:
	TEMA(OHLC* ohlc, int period) :Indicator(ohlc) { m_period = std::max(min_period, period); init(); m_name = "TEMA";};
	~TEMA() {};
	void init()
	{
		line.resize(m_ohlc->close.size(), 0.0);
		
		// Calculate first EMA
		std::vector<double> ema1(m_ohlc->close.size(), 0.0);
		double multiplier = 2.0 / (m_period + 1.0);
		
		// First EMA is SMA
		double sum = 0.0;
		for (int i = 0; i < m_period; i++)
		{
			sum += m_ohlc->close[i];
		}
		ema1[m_period - 1] = sum / m_period;

		// Calculate first EMA
		for (size_t i = m_period; i < m_ohlc->close.size(); i++)
		{
			ema1[i] = (m_ohlc->close[i] - ema1[i - 1]) * multiplier + ema1[i - 1];
		}

		// Calculate second EMA of the first EMA
		std::vector<double> ema2(m_ohlc->close.size(), 0.0);
		sum = 0.0;
		for (int i = 0; i < m_period; i++)
		{
			sum += ema1[i];
		}
		ema2[m_period - 1] = sum / m_period;

		for (size_t i = m_period; i < m_ohlc->close.size(); i++)
		{
			ema2[i] = (ema1[i] - ema2[i - 1]) * multiplier + ema2[i - 1];
		}

		// Calculate third EMA of the second EMA (EMA3)
		sum = 0.0;
		for (int i = 0; i < m_period; i++)
		{
			sum += ema2[i];
		}
		line[m_period - 1] = sum / m_period;

		for (size_t i = m_period; i < m_ohlc->close.size(); i++)
		{
			line[i] = (ema2[i] - line[i - 1]) * multiplier + line[i - 1];
		}

		// TEMA = 3*EMA1 - 3*EMA2 + EMA3
		for (size_t i = m_period - 1; i < m_ohlc->close.size(); i++)
		{
			line[i] = 3.0 * ema1[i] - 3.0 * ema2[i] + line[i];
		}

		init_period = m_period - 1;
	}
public:
	int min_period = 1;
	int m_period;
};

class RSI : public Indicator
{
public:
	RSI(OHLC* ohlc, int period) :Indicator(ohlc) { m_period = period; init(); m_name = "RSI"; isSubplot = true; };
	~RSI() {};
	void init()
	{
		line.resize(m_ohlc->close.size(), 50.0);
		if (m_period < min_period) return;

		std::vector<double> gains, losses;
		gains.resize(m_ohlc->close.size(), 0.0);
		losses.resize(m_ohlc->close.size(), 0.0);

		// Calculate price changes
		for (size_t i = 1; i < m_ohlc->close.size(); i++)
		{
			double change = m_ohlc->close[i] - m_ohlc->close[i - 1];
			if (change >= 0)
			{
				gains[i] = change;
				losses[i] = 0;
			}
			else
			{
				gains[i] = 0;
				losses[i] = -change;
			}
		}

		// Calculate average gains and losses
		double avgGain = 0.0, avgLoss = 0.0;
		for (int i = 1; i <= m_period; i++)
		{
			avgGain += gains[i];
			avgLoss += losses[i];
		}
		avgGain /= m_period;
		avgLoss /= m_period;

		// Calculate RSI
		for (size_t i = m_period + 1; i < m_ohlc->close.size(); i++)
		{
			avgGain = (avgGain * (m_period - 1) + gains[i]) / m_period;
			avgLoss = (avgLoss * (m_period - 1) + losses[i]) / m_period;
			
			if (avgLoss == 0)
				line[i] = 100;
			else
			{
				double rs = avgGain / avgLoss;
				line[i] = 100 - (100 / (1 + rs));
			}
		}
		init_period = m_period;
	}
public:
	int min_period = 2;
	int m_period;
};


class ADX : public Indicator
{
public:
	ADX(OHLC* ohlc, int period) :Indicator(ohlc) { m_period = std::max(min_period, period); init(); m_name = "ADX"; isSubplot = true;};
	~ADX() {};
	void init()
	{
		line.resize(m_ohlc->close.size(), 0.0);
		
		// Calculate True Range
		std::vector<double> tr(m_ohlc->close.size(), 0.0);
		for (size_t i = 1; i < m_ohlc->close.size(); i++)
		{
			double high = m_ohlc->high[i];
			double low = m_ohlc->low[i];
			double prevClose = m_ohlc->close[i - 1];
			
			tr[i] = std::max(high - low,
				std::max(std::abs(high - prevClose),
					std::abs(low - prevClose)));
		}

		// Calculate +DM and -DM
		std::vector<double> plusDM(m_ohlc->close.size(), 0.0);
		std::vector<double> minusDM(m_ohlc->close.size(), 0.0);
		
		for (size_t i = 1; i < m_ohlc->close.size(); i++)
		{
			double upMove = m_ohlc->high[i] - m_ohlc->high[i - 1];
			double downMove = m_ohlc->low[i - 1] - m_ohlc->low[i];
			
			if (upMove > downMove && upMove > 0)
				plusDM[i] = upMove;
			if (downMove > upMove && downMove > 0)
				minusDM[i] = downMove;
		}

		// Calculate smoothed TR, +DM, and -DM
		std::vector<double> smoothedTR(m_ohlc->close.size(), 0.0);
		std::vector<double> smoothedPlusDM(m_ohlc->close.size(), 0.0);
		std::vector<double> smoothedMinusDM(m_ohlc->close.size(), 0.0);

		// First values are simple sums
		double sumTR = 0.0, sumPlusDM = 0.0, sumMinusDM = 0.0;
		for (int i = 1; i <= m_period; i++)
		{
			sumTR += tr[i];
			sumPlusDM += plusDM[i];
			sumMinusDM += minusDM[i];
		}
		smoothedTR[m_period] = sumTR;
		smoothedPlusDM[m_period] = sumPlusDM;
		smoothedMinusDM[m_period] = sumMinusDM;

		// Calculate smoothed values
		for (size_t i = m_period + 1; i < m_ohlc->close.size(); i++)
		{
			smoothedTR[i] = smoothedTR[i - 1] - (smoothedTR[i - 1] / m_period) + tr[i];
			smoothedPlusDM[i] = smoothedPlusDM[i - 1] - (smoothedPlusDM[i - 1] / m_period) + plusDM[i];
			smoothedMinusDM[i] = smoothedMinusDM[i - 1] - (smoothedMinusDM[i - 1] / m_period) + minusDM[i];
		}

		// Calculate +DI and -DI
		std::vector<double> plusDI(m_ohlc->close.size(), 0.0);
		std::vector<double> minusDI(m_ohlc->close.size(), 0.0);

		for (size_t i = m_period; i < m_ohlc->close.size(); i++)
		{
			plusDI[i] = 100.0 * (smoothedPlusDM[i] / smoothedTR[i]);
			minusDI[i] = 100.0 * (smoothedMinusDM[i] / smoothedTR[i]);
		}

		// Calculate DX
		std::vector<double> dx(m_ohlc->close.size(), 0.0);
		for (size_t i = m_period; i < m_ohlc->close.size(); i++)
		{
			double sumDI = plusDI[i] + minusDI[i];
			dx[i] = (sumDI == 0.0) ? 0.0 : (100.0 * std::abs(plusDI[i] - minusDI[i]) / sumDI);
		}

		// ADX = Wilder's smoothing of DX (first value = avg of first period DX values)
		size_t adxStart = (size_t)(m_period * 2 - 1);
		if (m_ohlc->close.size() > adxStart)
		{
			double sumDX = 0.0;
			for (int i = m_period; i < m_period * 2 && i < (int)m_ohlc->close.size(); i++)
			{
				sumDX += dx[i];
			}
			line[adxStart] = sumDX / m_period;
			for (size_t i = adxStart + 1; i < m_ohlc->close.size(); i++)
			{
				line[i] = (line[i - 1] * (m_period - 1) + dx[i]) / m_period;
			}
		}

		init_period = (int)adxStart;
	}
public:
	int min_period = 2;
	int m_period;

};

class ATR : public Indicator
{
public:
	ATR(OHLC* ohlc, int period) :Indicator(ohlc) { m_period = std::max(min_period, period); init(); m_name = "ATR"; isSubplot = true; };
	~ATR() {};
	void init()
	{
		line.resize(m_ohlc->close.size(), 0.0);
		std::vector<double> trueRanges(m_ohlc->close.size(), 0.0);

		// Calculate True Range
		for (size_t i = 1; i < m_ohlc->close.size(); i++)
		{
			double high = m_ohlc->high[i];
			double low = m_ohlc->low[i];
			double prevClose = m_ohlc->close[i - 1];
			
			trueRanges[i] = std::max(high - low,
				std::max(std::abs(high - prevClose),
					std::abs(low - prevClose)));
		}

		// Calculate ATR using SMA
		double sum = 0.0;
		for (int i = 1; i <= m_period; i++)
		{
			sum += trueRanges[i];
		}
		line[m_period] = sum / m_period;

		for (size_t i = m_period + 1; i < m_ohlc->close.size(); i++)
		{
			line[i] = (line[i - 1] * (m_period - 1) + trueRanges[i]) / m_period;
		}
		init_period = m_period;
	}
public:
	int min_period = 1;
	int m_period;

};

class AverageRage : public Indicator
{
public:
	AverageRage(OHLC* ohlc, int period) :Indicator(ohlc) { m_period = std::max(min_period, period); init(); m_name = "AverageRage"; isSubplot = true; };
	~AverageRage() {};
	void init()
	{
		line.resize(m_ohlc->close.size(), 0.0);
		std::vector<double> ranger(m_ohlc->close.size(), 0.0);
		for (size_t i = 0; i < m_ohlc->close.size(); i++)
		{
			if (m_ohlc->open[i] == 0.0)
				ranger[i] = 0.0;
			else
				ranger[i] = 100.0 * (m_ohlc->high[i] - m_ohlc->low[i]) / m_ohlc->open[i];
		}

		double rolling = 0.0;
		for (size_t i = 0; i < m_ohlc->close.size(); i++)
		{
			rolling += ranger[i];
			if (i >= (size_t)m_period)
				rolling -= ranger[i - m_period];

			if (i + 1 >= (size_t)m_period)
				line[i] = rolling / m_period;
			else
				line[i] = rolling / (i + 1);
		}
		init_period = m_period - 1;
	}
public:
	int min_period = 1;
	int m_period;
};

class NormalizedRange : public Indicator
{
public:
	NormalizedRange(OHLC* ohlc, int period) :Indicator(ohlc) { m_period = std::max(min_period, period); init(); m_name = "NormalizedRange"; isSubplot = true; };
	~NormalizedRange() {};
	void init()
	{
		line.resize(m_ohlc->close.size(), 0.0);
		std::vector<double> ranger(m_ohlc->close.size(), 0.0);
		for (size_t i = 0; i < m_ohlc->close.size(); i++)
		{
			if (m_ohlc->open[i] == 0.0)
				ranger[i] = 0.0;
			else
				ranger[i] = 100.0 * std::abs(m_ohlc->close[i] - m_ohlc->open[i]) / m_ohlc->open[i];
		}

		double rolling = 0.0;
		for (size_t i = 0; i < m_ohlc->close.size(); i++)
		{
			rolling += ranger[i];
			if (i >= (size_t)m_period)
				rolling -= ranger[i - m_period];

			if (i + 1 >= (size_t)m_period)
				line[i] = rolling / m_period;
			else
				line[i] = rolling / (i + 1);
		}
		init_period = m_period - 1;
	}
public:
	int min_period = 1;
	int m_period;
};

class AverageDiff : public Indicator
{
public:
	AverageDiff(OHLC* ohlc, int period) :Indicator(ohlc) { m_period = std::max(min_period, period); init(); m_name = "AverageDiff"; isSubplot = true; };
	~AverageDiff() {};
	void init()
	{
		line.resize(m_ohlc->close.size(), 0.0);
		atdLine.resize(m_ohlc->close.size(), 0.0);

		TEMA tema(m_ohlc, m_period);

		std::vector<double> range(m_ohlc->close.size(), 0.0);
		for (size_t i = 0; i < m_ohlc->close.size(); i++)
			range[i] = m_ohlc->high[i] - m_ohlc->low[i];

		std::vector<double> rangeEma(m_ohlc->close.size(), 0.0);
		if (!range.empty())
		{
			const int p = std::max(1, m_period);
			const double mult = 2.0 / (p + 1.0);
			double sum = 0.0;
			const int seedEnd = std::min<int>(p, (int)range.size());
			for (int i = 0; i < seedEnd; i++)
				sum += range[i];
			rangeEma[seedEnd - 1] = sum / seedEnd;
			for (size_t i = (size_t)seedEnd; i < range.size(); i++)
				rangeEma[i] = (range[i] - rangeEma[i - 1]) * mult + rangeEma[i - 1];
		}

		for (size_t i = 0; i < m_ohlc->close.size(); i++)
		{
			double diff = m_ohlc->close[i] - tema.line[i];
			double denClose = m_ohlc->close[i];
			double denAtr = rangeEma[i];
			line[i] = denClose == 0.0 ? 0.0 : (100.0 * diff / denClose);
			atdLine[i] = denAtr == 0.0 ? 0.0 : (diff / denAtr);
		}
		init_period = std::max(tema.init_period, m_period - 1);
	}
public:
	int min_period = 1;
	int m_period;
	std::vector<double> atdLine;
};

class ATD : public Indicator
{
public:
	ATD(OHLC* ohlc, int period, int atrPeriod = -1) :Indicator(ohlc)
	{
		m_period = std::max(min_period, period);
		m_atrPeriod = atrPeriod > 0 ? std::max(min_period, atrPeriod) : m_period;
		init();
		m_name = "ATD";
		isSubplot = true;
	};
	~ATD() {};
	void init()
	{
		line.resize(m_ohlc->close.size(), 0.0);

		TEMA tema(m_ohlc, m_period);
		std::vector<double> range(m_ohlc->close.size(), 0.0);
		for (size_t i = 0; i < m_ohlc->close.size(); i++)
			range[i] = m_ohlc->high[i] - m_ohlc->low[i];

		std::vector<double> rangeEma(m_ohlc->close.size(), 0.0);
		if (!range.empty())
		{
			const int p = std::max(1, m_atrPeriod);
			const double mult = 2.0 / (p + 1.0);
			double sum = 0.0;
			const int seedEnd = std::min<int>(p, (int)range.size());
			for (int i = 0; i < seedEnd; i++)
				sum += range[i];
			rangeEma[seedEnd - 1] = sum / seedEnd;
			for (size_t i = (size_t)seedEnd; i < range.size(); i++)
				rangeEma[i] = (range[i] - rangeEma[i - 1]) * mult + rangeEma[i - 1];
		}

		for (size_t i = 0; i < m_ohlc->close.size(); i++)
		{
			double diff = m_ohlc->close[i] - tema.line[i];
			double den = rangeEma[i];
			line[i] = den == 0.0 ? 0.0 : (diff / den);
		}
		init_period = std::max(tema.init_period, m_atrPeriod - 1);
	}
public:
	int min_period = 1;
	int m_period;
	int m_atrPeriod;
};

class RelativeRange : public Indicator
{
public:
	RelativeRange(OHLC* ohlc, int smoothingLength, int rangeLength) :Indicator(ohlc)
	{
		m_smoothingLength = std::max(min_period, smoothingLength);
		m_rangeLength = std::max(min_period, rangeLength);
		init();
		m_name = "RelativeRange";
		isSubplot = true;
	};
	~RelativeRange() {};
	void init()
	{
		line.resize(m_ohlc->close.size(), 0.0);
		std::vector<double> absRange(m_ohlc->close.size(), 0.0);
		for (size_t i = 0; i < m_ohlc->close.size(); i++)
		{
			absRange[i] = std::abs(m_ohlc->close[i] - m_ohlc->open[i]);
		}

		std::vector<double> avgRange(m_ohlc->close.size(), 0.0);
		double rolling = 0.0;
		for (size_t i = 0; i < m_ohlc->close.size(); i++)
		{
			rolling += absRange[i];
			if (i >= (size_t)m_rangeLength)
				rolling -= absRange[i - m_rangeLength];
			if (i + 1 >= (size_t)m_rangeLength)
				avgRange[i] = rolling / m_rangeLength;
			else
				avgRange[i] = rolling / (i + 1);
		}

		std::vector<double> rrRaw(m_ohlc->close.size(), 0.0);
		for (size_t i = 0; i < m_ohlc->close.size(); i++)
		{
			rrRaw[i] = avgRange[i] == 0.0 ? 0.0 : (100.0 * (m_ohlc->close[i] - m_ohlc->open[i]) / avgRange[i]);
		}

		rolling = 0.0;
		for (size_t i = 0; i < m_ohlc->close.size(); i++)
		{
			rolling += rrRaw[i];
			if (i >= (size_t)m_smoothingLength)
				rolling -= rrRaw[i - m_smoothingLength];
			if (i + 1 >= (size_t)m_smoothingLength)
				line[i] = rolling / m_smoothingLength;
			else
				line[i] = rolling / (i + 1);
		}

		init_period = std::max(m_smoothingLength, m_rangeLength) - 1;
	}
public:
	int min_period = 1;
	int m_smoothingLength;
	int m_rangeLength;
};

class EWO : public Indicator
{
public:
	EWO(OHLC* ohlc, int fper, int sper) :Indicator(ohlc)
	{
		m_fastPeriod = std::max(min_period, fper);
		m_slowPeriod = std::max(min_period, sper);
		init();
		m_name = "EWO";
		isSubplot = true;
	};
	~EWO() {};
	void init()
	{
		line.resize(m_ohlc->close.size(), 0.0);
		EMA fastEma(m_ohlc, m_fastPeriod);
		EMA slowEma(m_ohlc, m_slowPeriod);

		for (size_t i = 0; i < m_ohlc->close.size(); i++)
		{
			double close = m_ohlc->close[i];
			line[i] = close == 0.0 ? 0.0 : ((fastEma.line[i] - slowEma.line[i]) / close * 100.0);
		}

		init_period = std::max(fastEma.init_period, slowEma.init_period);
	}
public:
	int min_period = 1;
	int m_fastPeriod;
	int m_slowPeriod;
};

class TD9 : public Indicator
{
public:
	TD9(OHLC* ohlc) :Indicator(ohlc) {init();m_name = "TD9"; isSubplot = true;};
	~TD9() {};
	void init()
	{
		auto     closeLen = m_ohlc->close.size();
		init_period = 10;

		for (size_t i =0 ; i < init_period; i++)
		{
			line.push_back(0);
		}

		for (size_t i = init_period; i < closeLen; i++)
		{
			int res = 0;
			if (m_ohlc->high[i - 4] < m_ohlc->close[i])
			{
				res = (int)line[i - 1] + 1;
			}
			else if (m_ohlc->low[i - 4] > m_ohlc->close[i])
			{
				res = (int)line[i - 1] - 1;
			}
			else if (line[i - 1] > 0 || line[i - 1] < 0)
			{
				res = 0;
			}

			line.push_back(res);
		}
	}
};

class CrossOver : public Indicator
{
public:
	CrossOver(OHLC* ohlc, std::vector<double>* line1, std::vector<double>* line2) :m_line1(*line1), m_line2(*line2), Indicator(ohlc) { init();m_name = "CrossOver";  isSubplot = true;};
	~CrossOver() {};
	void init()
	{
		auto closeLen = m_ohlc->close.size();
		line.push_back(0);

		for (size_t i = 1; i < closeLen; i++)
		{
			short res = 0;
			if ((m_line1[i] > m_line2[i]) && (m_line1[i - 1] <= m_line2[i - 1]))
			{
				res = 1;
			}

			if ((m_line1[i] < m_line2[i]) && (m_line1[i - 1] >= m_line2[i - 1]))
			{
				res = -1;
			}

			line.push_back(res);
		}

	}
	std::vector<double>& m_line1;
	std::vector<double>& m_line2;
};

class SuperTrendBand : public Indicator
{
public:
	SuperTrendBand(OHLC* ohlc, int period, double multiplier) :Indicator(ohlc)
	{
		m_period = std::max(min_period, period);
		m_multiplier = multiplier;
		init();
		m_name = "SuperTrendBand";
	};
	~SuperTrendBand() {};
	void init()
	{
		const size_t len = m_ohlc->close.size();
		basic_ub.resize(len, 0.0);
		basic_lb.resize(len, 0.0);
		final_ub.resize(len, 0.0);
		final_lb.resize(len, 0.0);
		line.resize(len, 0.0);

		ATR atr(m_ohlc, m_period);
		init_period = atr.init_period;

		for (size_t i = 0; i < len; i++)
		{
			basic_ub[i] = ((m_ohlc->high[i] + m_ohlc->low[i]) / 2.0) + (atr.line[i] * m_multiplier);
			basic_lb[i] = ((m_ohlc->high[i] + m_ohlc->low[i]) / 2.0) - (atr.line[i] * m_multiplier);
		}

		if (len == 0)
			return;

		final_ub[0] = basic_ub[0];
		final_lb[0] = basic_lb[0];
		line[0] = final_ub[0];
		for (size_t i = 1; i < len; i++)
		{
			final_ub[i] = (basic_ub[i] < final_ub[i - 1] || m_ohlc->close[i - 1] > final_ub[i - 1]) ? basic_ub[i] : final_ub[i - 1];
			final_lb[i] = (basic_lb[i] > final_lb[i - 1] || m_ohlc->close[i - 1] < final_lb[i - 1]) ? basic_lb[i] : final_lb[i - 1];
			line[i] = final_ub[i];
		}
	}
public:
	int min_period = 1;
	int m_period;
	double m_multiplier;
	std::vector<double> basic_ub;
	std::vector<double> basic_lb;
	std::vector<double> final_ub;
	std::vector<double> final_lb;
};

class SuperTrend : public Indicator
{
public:
	SuperTrend(OHLC* ohlc, int period, double multi) :Indicator(ohlc) { m_period = period; m_multi = std::max(0.001, multi); init(); m_name = "SuperTrend";};
	~SuperTrend() {};
	void init()
	{

		size_t closeLen               = m_ohlc->close.size();
		ATR atr(m_ohlc, m_period);
		std::vector<double>& atr_line = atr.line;
		std::vector<double>& close    = m_ohlc->close;
		std::vector<double>& high     = m_ohlc->high;
		std::vector<double>& low      = m_ohlc->low;


		init_period = atr.init_period;

		double upper_band = (high[0] + low[0]) / 2 + (m_multi * atr_line[0]);
		double lower_band = (high[0] + low[0]) / 2 - (m_multi * atr_line[0]);

		double prev_final_upper_band = upper_band;
		double prev_final_lower_band = lower_band;

		double prev_superTrend = upper_band;


		if (m_period < min_period)
		{
			for (size_t i = 0; i < closeLen; i++)
			{
				line.push_back(0);
			}
			return;
		}

		line.push_back(upper_band);
		for (size_t i = 1; i < closeLen; i++)
		{
			
			double upper_band = ((high[i] + low[i]) / 2) + (m_multi * atr_line[i]);
			double lower_band = ((high[i] + low[i]) / 2) - (m_multi * atr_line[i]);

			double final_upper_band = upper_band;
			double final_lower_band = lower_band;

			if ((upper_band < prev_final_upper_band) || (close[i-1] > prev_final_upper_band))
			{
				final_upper_band=upper_band;
			}
			else
			{
				final_upper_band=prev_final_upper_band;
			}
			
			if ((lower_band > prev_final_lower_band) || (close[i-1] < prev_final_lower_band))
			{
				final_lower_band=lower_band;
			}
			else
			{
				final_lower_band=prev_final_lower_band;
			}

			double super_trend = final_upper_band;
			if (prev_superTrend == prev_final_upper_band)
			{
				if (close[i] <= final_upper_band)
				{
					super_trend = final_upper_band;
				}
				else
				{
					super_trend = final_lower_band;
				}
			}

			if (prev_superTrend == prev_final_lower_band)
			{
				if (close[i] >= final_lower_band)
				{
					super_trend = final_lower_band;
				}
				else
				{
					super_trend = final_upper_band;
				}
			}

			line.push_back(super_trend);

			prev_final_upper_band = final_upper_band;
			prev_final_lower_band = final_lower_band;
			prev_superTrend = super_trend;

		}
	}
public:
	int min_period = 2;
	int m_period;
	double m_multi;
};


class BolingerBand : public Indicator
{
public:
	BolingerBand(OHLC* ohlc, int period, double diff) :Indicator(ohlc) { m_period = period; m_diff = diff; init(); m_name = "BolingerBand"; };
	~BolingerBand() {};
	void init()
	{
		line.resize(m_ohlc->close.size(), 0.0);
		top.resize(m_ohlc->close.size(), 0.0);
		bot.resize(m_ohlc->close.size(), 0.0);

		if (m_period < min_period)
		{
			return;
		}

		// Calculate SMA
		for (size_t i = m_period - 1; i < m_ohlc->close.size(); i++)
		{
			double sum = 0.0;
			for (int j = 0; j < m_period; j++)
			{
				sum += m_ohlc->close[i - j];
			}
			line[i] = sum / m_period;

			// Calculate standard deviation
			double sumSquaredDiff = 0.0;
			for (int j = 0; j < m_period; j++)
			{
				double diff = m_ohlc->close[i - j] - line[i];
				sumSquaredDiff += diff * diff;
			}
			double stdDev = std::sqrt(sumSquaredDiff / m_period);

			// Calculate bands
			top[i] = line[i] + (m_diff * stdDev);
			bot[i] = line[i] - (m_diff * stdDev);
		}

		init_period = m_period - 1;
	}
public:
	int min_period = 2;
	int m_period;
	double m_diff;
	std::vector<double> top;
	std::vector<double> bot;
};