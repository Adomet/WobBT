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
		auto     closeLen = m_ohlc->close.size();
		double* out = new double[closeLen];
		int     outBeg = 0;
		int     outNbElement = 0;

		auto retCode = TA_MA(0, closeLen - 1, &m_ohlc->close[0], m_period, TA_MAType_SMA, &outBeg, &outNbElement, &out[0]);

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
	EMA(OHLC* ohlc, int period) :Indicator(ohlc) { m_period = std::max(min_period, period); init(); m_name = "EMA";};
	~EMA() {};
	void init()
	{
		auto     closeLen = m_ohlc->close.size();
		double* out = new double[closeLen];
		int     outBeg = 0;
		int     outNbElement = 0;

		auto retCode = TA_MA(0, closeLen - 1, &m_ohlc->close[0], m_period, TA_MAType_EMA, &outBeg, &outNbElement, &out[0]);

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
	DEMA(OHLC* ohlc, int period) :Indicator(ohlc) { m_period = std::max(min_period, period); init(); m_name = "DEMA"; };
	~DEMA() {};
	void init()
	{
		auto     closeLen = m_ohlc->close.size();
		double* out = new double[closeLen];
		int     outBeg = 0;
		int     outNbElement = 0;

		auto retCode = TA_MA(0, closeLen - 1, &m_ohlc->close[0], m_period, TA_MAType_DEMA, &outBeg, &outNbElement, &out[0]);

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
	TEMA(OHLC* ohlc, int period) :Indicator(ohlc) { m_period = std::max(min_period, period); init(); m_name = "TEMA";};
	~TEMA() {};
	void init()
	{
		auto     closeLen = m_ohlc->close.size();
		double* out = new double[closeLen];
		int     outBeg = 0;
		int     outNbElement = 0;

		auto retCode = TA_MA(0, closeLen - 1, &m_ohlc->close[0], m_period, TA_MAType_TEMA, &outBeg, &outNbElement, &out[0]);

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
	RSI(OHLC* ohlc, int period) :Indicator(ohlc) { m_period = period; init(); m_name = "RSI"; isSubplot = true; };
	~RSI() {};
	void init()
	{
		auto     closeLen = m_ohlc->close.size();
		double* out = new double[closeLen];
		int     outBeg = 0;
		int     outNbElement = 0;

		auto retCode = TA_RSI(0, closeLen - 1, &m_ohlc->close[0], m_period, &outBeg, &outNbElement, &out[0]);

		if (m_period < min_period)
		{
			for (size_t i = 0; i < closeLen; i++)
			{
				line.push_back(0);
			}
			return;
		}

		for (int i = 0; i < outBeg; i++)
		{
			line.push_back(50);
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
	ADX(OHLC* ohlc, int period) :Indicator(ohlc) { m_period = std::max(min_period, period); init(); m_name = "ADX"; isSubplot = true;};
	~ADX() {};
	void init()
	{
		auto     closeLen = m_ohlc->close.size();
		double* out = new double[closeLen];
		int     outBeg = 0;
		int     outNbElement = 0;

		auto retCode = TA_ADX(0, closeLen - 1, &m_ohlc->high[0], &m_ohlc->low[0], &m_ohlc->close[0], m_period, &outBeg, &outNbElement, &out[0]);

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
	ATR(OHLC* ohlc, int period) :Indicator(ohlc) { m_period = std::max(min_period, period); init();m_name = "ATR"; isSubplot = true; };
	~ATR() {};
	void init()
	{
		auto     closeLen = m_ohlc->close.size();
		double* out = new double[closeLen];
		int     outBeg = 0;
		int     outNbElement = 0;

		auto retCode = TA_ATR(0, closeLen - 1, &m_ohlc->high[0], &m_ohlc->low[0], &m_ohlc->close[0], m_period, &outBeg, &outNbElement, &out[0]);

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
			double prvcandleclose = 0;
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

class SuperTrend : public Indicator
{
public:
	SuperTrend(OHLC* ohlc, int period, double multi) :Indicator(ohlc) { m_period = period; m_multi = std::max(0.001, multi); init(); m_name = "SuperTrend"; isSubplot = true;};
	~SuperTrend() {};
	void init()
	{

		size_t closeLen               = m_ohlc->close.size();
		Indicator& atr                = ATR(m_ohlc, m_period);
		std::vector<double>& atr_line = atr.line;
		std::vector<double>& close    = m_ohlc->close;
		std::vector<double>& high     = m_ohlc->high;
		std::vector<double>& low      = m_ohlc->low;


		init_period = atr.init_period;
		line.push_back(0);

		double upper_band = (high[0] + low[0]) / 2 + (m_multi * atr_line[0]);
		double lower_band = (high[0] + low[0]) / 2 - (m_multi * atr_line[0]);

		double prev_final_upper_band = upper_band;
		double prev_final_lower_band = upper_band;

		if (m_period < min_period)
		{
			for (size_t i = 0; i < closeLen; i++)
			{
				line.push_back(0);
			}
			return;
		}

		for (size_t i = 0; i < init_period; i++)
		{
			line.push_back(prev_final_upper_band);
		}
		for (size_t i = init_period; i < closeLen; i++)
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
			if (line[i] == prev_final_upper_band)
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

			if (line[i] == prev_final_lower_band)
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

		}
	}
public:
	int min_period = 2;
	int m_period;
	double m_multi;
};