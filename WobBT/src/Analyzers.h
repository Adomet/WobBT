#pragma once
#include "Strategy.h"

class General :public Analyzer
{
public:
	General(Strategy& strat):m_strat(strat) { { m_name = "General"; } };
	~General() = default;

	double run()
	{
		return m_Result;
	};
public:
	Strategy& m_strat;
};


class WinStreaks :public Analyzer
{
public:
	WinStreaks(Strategy& strat) :m_strat(strat) { m_name = "WinStreaks"; };
	~WinStreaks() = default;

	double run()
	{
		m_Result = m_strat.m_max_winStreak;
		return m_Result;
	};
public:
	Strategy& m_strat;
};

class LoseStreaks :public Analyzer
{
public:
	LoseStreaks(Strategy& strat) :m_strat(strat) { m_name = "LoseStreaks"; };
	~LoseStreaks() = default;

	double run()
	{
		m_Result = m_strat.m_max_loseStreak;
		return m_Result;
	};
public:
	Strategy& m_strat;
};

class ReturnPNL :public Analyzer
{
public:
	ReturnPNL(Strategy& strat) :m_strat(strat) { m_name = "ReturnPNL"; };
	~ReturnPNL() = default;

	double run()
	{
		auto len = m_strat.m_eqQurve.size();
		m_Result = m_strat.m_eqQurve[len-1];
		return m_Result;
	};
public:
	Strategy& m_strat;
};


class WinRate :public Analyzer
{
public:
	WinRate(Strategy& strat) :m_strat(strat) { m_name = "WinRate"; };
	~WinRate() = default;

	double run()
	{
		m_Result = (((double)m_strat.m_winTradeCount) / ((double)m_strat.m_tradeCount));
		return m_Result;
	};
public:
	Strategy& m_strat;
};

class MaxDD :public Analyzer
{
public:
	MaxDD(Strategy& strat) :m_strat(strat) { m_name = "MaxDD"; };
	~MaxDD() = default;

	double run()
	{
		auto data = m_strat.m_eqQurve;

		size_t dataSize = data.size();

		double maxRet = data[0];
		double maxDD  = 0;

		for (size_t i = 0; i < dataSize; i++)
		{
			if (maxRet < data[i])
			{
				maxRet = data[i];
			}

			double DD = (maxRet - data[i])/maxRet * 100.0;

			if (maxDD < DD)
			{
				maxDD = DD;
			}
		}
		
		m_Result = maxDD;

		return m_Result;
	};
public:
	Strategy& m_strat;
};



class SQN :public Analyzer
{
public:
	SQN(Strategy& strat):m_strat(strat) { m_name = "SQN"; };
	~SQN()= default;

	double run()
	{
		auto data = m_strat.m_PNL;
		size_t dataSize = data.size();

		double sum  = 0.0;
		for (size_t i = 0; i < dataSize; i++)
		{
			sum += data[i];
		}

		double mean = sum / dataSize;

		double SD = 0.0;
		for (size_t i = 0; i < dataSize; i++)
		{
			SD += pow(data[i] - mean, 2);
		}

		m_Result = (mean * sqrt(dataSize)) / sqrt(SD/dataSize);
		return m_Result;
	};
public:
	Strategy& m_strat;
};
