#pragma once
#include "OHLC.h"
#include "ta_libc.h"
#include <vector>
#include <iostream>
#include <deque>

//Indicator List: SMA,EMA,DEMA,TEMA,RSI,ATR,ADX,TD9,CrossOver,SuperTrend

class Indicator
{
public:
	Indicator(OHLC* ohlc) :m_ohlc(ohlc) {};
	~Indicator() {};
	virtual void init() {};


public:
	std::vector<double> line;
	OHLC* m_ohlc;
	int init_period = 0;
};

class SMA : public Indicator
{
public:
	SMA(OHLC* ohlc, int period):Indicator(ohlc) { m_period = std::max(min_period, period); init(); };
	~SMA() {};
	void init()
	{
		auto     closeLen = m_ohlc->close.size();
		double* out = new double[closeLen];
		int     outBeg = 0;
		int     outNbElement = 0;

		auto retCode = TA_S_MA(0, closeLen - 1, &m_ohlc->close[0], m_period, TA_MAType_SMA, &outBeg, &outNbElement, &out[0]);

		for (int i = 0; i < outBeg; i++)
		{
			line.push_back(0);
		}

		for (int i = 0; i < outNbElement; i++)
		{
				line.push_back(out[i]);
		}
			//printf("Candle %d = %f\n", 1 + outBeg + i, out[i]);

		init_period = outBeg;

		delete[closeLen] out;
	}
public:
	int min_period = 1;
	int m_period;
};

class EMA : public Indicator
{
public:
	EMA(OHLC* ohlc, int period) :Indicator(ohlc) { m_period = std::max(min_period, period); init(); };
	~EMA() {};
	void init()
	{
		auto     closeLen = m_ohlc->close.size();
		double* out = new double[closeLen];
		int     outBeg = 0;
		int     outNbElement = 0;

		auto retCode = TA_S_MA(0, closeLen - 1, &m_ohlc->close[0], m_period, TA_MAType_EMA, &outBeg, &outNbElement, &out[0]);

		for (int i = 0; i < outBeg; i++)
		{
			line.push_back(0);
		}

		for (int i = 0; i < outNbElement; i++)
		{
			line.push_back(out[i]);
		}
		//printf("Candle %d = %f\n", 1 + outBeg + i, out[i]);

		init_period = outBeg;

		delete[closeLen] out;
	}
public:
	int min_period = 1;
	int m_period;
};

class DEMA : public Indicator
{
public:
	DEMA(OHLC* ohlc, int period) :Indicator(ohlc) { m_period = std::max(min_period, period); init(); };
	~DEMA() {};
	void init()
	{
		auto     closeLen = m_ohlc->close.size();
		double* out = new double[closeLen];
		int     outBeg = 0;
		int     outNbElement = 0;

		auto retCode = TA_S_MA(0, closeLen - 1, &m_ohlc->close[0], m_period, TA_MAType_DEMA, &outBeg, &outNbElement, &out[0]);

		for (int i = 0; i < outBeg; i++)
		{
			line.push_back(0);
		}

		for (int i = 0; i < outNbElement; i++)
		{
			line.push_back(out[i]);
		}
		//printf("Candle %d = %f\n", 1 + outBeg + i, out[i]);
		init_period = outBeg;

		delete[closeLen] out;
	}
public:
	int min_period = 1;
	int m_period;
};


class TEMA : public Indicator
{
public:
	TEMA(OHLC* ohlc, int period) :Indicator(ohlc) { m_period = std::max(min_period, period); init(); };
	~TEMA() {};
	void init()
	{
		auto     closeLen = m_ohlc->close.size();
		double* out = new double[closeLen];
		int     outBeg = 0;
		int     outNbElement = 0;

		auto retCode = TA_S_MA(0, closeLen - 1, &m_ohlc->close[0], m_period, TA_MAType_TEMA, &outBeg, &outNbElement, &out[0]);

		for (int i = 0; i < outBeg; i++)
		{
			line.push_back(0);
		}

		for (int i = 0; i < outNbElement; i++)
		{
			line.push_back(out[i]);
		}
		//printf("Candle %d = %f\n", 1 + outBeg + i, out[i]);
		init_period = outBeg;

		delete[closeLen] out;
	}
public:
	int min_period = 1;
	int m_period;
};

class RSI : public Indicator
{
public:
	RSI(OHLC* ohlc, int period) :Indicator(ohlc) { m_period = std::max(min_period,period); init(); };
	~RSI() {};
	void init()
	{
		auto     closeLen = m_ohlc->close.size();
		double* out = new double[closeLen];
		int     outBeg = 0;
		int     outNbElement = 0;

		auto retCode = TA_S_RSI(0, closeLen - 1, &m_ohlc->close[0], m_period, &outBeg, &outNbElement, &out[0]);

		for (int i = 0; i < outBeg; i++)
		{
			line.push_back(0);
		}

		for (int i = 0; i < outNbElement; i++)
		{
			line.push_back(out[i]);
		}
		//printf("Candle %d = %f\n", 1 + outBeg + i, out[i]);
		init_period = outBeg;

		delete[closeLen] out;
	}
public:
	int min_period = 2;
	int m_period;

};


class ADX : public Indicator
{
public:
	ADX(OHLC* ohlc, int period) :Indicator(ohlc) { m_period = std::max(min_period, period); init(); };
	~ADX() {};
	void init()
	{
		auto     closeLen = m_ohlc->close.size();
		double* out = new double[closeLen];
		int     outBeg = 0;
		int     outNbElement = 0;

		auto retCode = TA_S_ADX(0, closeLen - 1, &m_ohlc->high[0], &m_ohlc->low[0], &m_ohlc->close[0], m_period, &outBeg, &outNbElement, &out[0]);

		for (int i = 0; i < outBeg; i++)
		{
			line.push_back(0);
		}

		for (int i = 0; i < outNbElement; i++)
		{
			line.push_back(out[i]);
		}
		//printf("Candle %d = %f\n", 1 + outBeg + i, out[i]);
		init_period = outBeg;

		delete[closeLen] out;
	}
public:
	int min_period = 2;
	int m_period;

};

class ATR : public Indicator
{
public:
	ATR(OHLC* ohlc, int period) :Indicator(ohlc) { m_period = std::max(min_period, period); init(); };
	~ATR() {};
	void init()
	{
		auto     closeLen = m_ohlc->close.size();
		double* out = new double[closeLen];
		int     outBeg = 0;
		int     outNbElement = 0;

		auto retCode = TA_S_ATR(0, closeLen - 1, &m_ohlc->high[0], &m_ohlc->low[0], &m_ohlc->close[0], m_period, &outBeg, &outNbElement, &out[0]);

		for (int i = 0; i < outBeg; i++)
		{
			line.push_back(0);
		}

		for (int i = 0; i < outNbElement; i++)
		{
			line.push_back(out[i]);
		}
		//printf("Candle %d = %f\n", 1 + outBeg + i, out[i]);
		init_period = outBeg;

		delete[closeLen] out;
	}
public:
	int min_period = 2;
	int m_period;

};

class TD9 : public Indicator
{
public:
	TD9(OHLC* ohlc) :Indicator(ohlc) {init();};
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
			float prvcandleclose = 0;
			if (m_ohlc->high[i - 4] < m_ohlc->close[i])
			{
				prvcandleclose = m_ohlc->close[i];
				res = line[i - 1]+1;
			}
			else if (line[i - 1] > 0)
			{
				res = 0;
			}

			if (m_ohlc->low[i - 4] > m_ohlc->close[i])
			{
				prvcandleclose = m_ohlc->close[i];
				res = line[i - 1] - 1;
			}
			else if (line[i - 1] < 0)
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
	CrossOver(OHLC* ohlc, std::vector<double>* line1, std::vector<double>* line2) :m_line1(*line1), m_line2(*line2), Indicator(ohlc) { init(); };
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

class SuperTrend : public Indicator
{
public:
	SuperTrend(OHLC* ohlc, int period,double multi) :Indicator(ohlc) { m_period = std::max(min_period, period); m_multi = std::max(0.001, multi); init(); };
	~SuperTrend() {};
	void init()
	{
		//Debug::Log("m_period" + std::to_string(m_period) + " m_multi" + std::to_string(m_multi));

		Indicator& atr = ATR(m_ohlc, m_period);
		std::vector<double>& atr_line = atr.line;
		size_t closeLen = m_ohlc->close.size();
		std::vector<float>& close    = m_ohlc->close;
		std::vector<float>& high     = m_ohlc->high;
		std::vector<float>& low      = m_ohlc->low;


		init_period = 1;
		line.push_back(0);
		double upper_band = (high[0] + low[0]) / 2 + (m_multi * atr_line[0]);
		double lower_band = (high[0] + low[0]) / 2 - (m_multi * atr_line[0]);
		double prev_final_upper_band = upper_band;
		double prev_final_lower_band = upper_band;


		for (size_t i = 0; i < atr.init_period; i++)
		{
			line.push_back(close[i]);
		}
		for (size_t i = atr.init_period; i < closeLen; i++)
		{
			double super_trend = 0;
			double upper_band = (high[i] + low[i]) / 2 + (m_multi * atr_line[i]);
			double lower_band = (high[i] + low[i]) / 2 - (m_multi * atr_line[i]);
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
			
			if ((close[i] < final_upper_band))
			{
				super_trend = final_upper_band;
			}
			else
			{
				super_trend = final_lower_band;
			}


			line.push_back(super_trend);

			prev_final_upper_band = final_upper_band;
			prev_final_lower_band = final_lower_band;

		}
	}
public:
	int min_period = 2;
	int m_period;
	double m_multi;
};