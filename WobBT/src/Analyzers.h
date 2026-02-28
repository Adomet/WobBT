#pragma once
#include "Strategy.h"
#include <cmath>

class TotalClosed : public Analyzer
{
public:
	static constexpr const char* name = "TotalClosed";
	TotalClosed(Strategy& strat) : m_strat(strat) { m_name = name; }
	~TotalClosed() = default;

	double run()
	{
		m_Result = static_cast<double>(m_strat.m_tradeCount);
		return m_Result;
	}
public:
	Strategy& m_strat;
};

class TotalWon : public Analyzer
{
public:
	static constexpr const char* name = "TotalWon";
	TotalWon(Strategy& strat) : m_strat(strat) { m_name = name; }
	~TotalWon() = default;

	double run()
	{
		m_Result = static_cast<double>(m_strat.m_winTradeCount);
		return m_Result;
	}
public:
	Strategy& m_strat;
};

class TotalLost : public Analyzer
{
public:
	static constexpr const char* name = "TotalLost";
	TotalLost(Strategy& strat) : m_strat(strat) { m_name = name; }
	~TotalLost() = default;

	double run()
	{
		m_Result = static_cast<double>(m_strat.m_tradeCount - m_strat.m_winTradeCount);
		return m_Result;
	}
public:
	Strategy& m_strat;
};

class WinStreaks : public Analyzer
{
public:
	static constexpr const char* name = "WinStreaks";
	WinStreaks(Strategy& strat) : m_strat(strat) { m_name = name; }
	~WinStreaks() = default;

	double run()
	{
		m_Result = m_strat.m_max_winStreak;
		return m_Result;
	};
public:
	Strategy& m_strat;
};

class LoseStreaks : public Analyzer
{
public:
	static constexpr const char* name = "LoseStreaks";
	LoseStreaks(Strategy& strat) : m_strat(strat) { m_name = name; }
	~LoseStreaks() = default;

	double run()
	{
		m_Result = m_strat.m_max_loseStreak;
		return m_Result;
	};
public:
	Strategy& m_strat;
};

class ReturnPNL : public Analyzer
{
public:
	static constexpr const char* name = "ReturnPNL";
	ReturnPNL(Strategy& strat) : m_strat(strat) { m_name = name; }
	~ReturnPNL() = default;

	double run()
	{
		auto len = m_strat.m_equityCurve.size();
		if (len == 0)
		{
			m_Result = 0;
			return m_Result;
		}
		m_Result = m_strat.m_equityCurve[len - 1];
		return m_Result;
	};
public:
	Strategy& m_strat;
};


class WinRate : public Analyzer
{
public:
	static constexpr const char* name = "WinRate";
	WinRate(Strategy& strat) : m_strat(strat) { m_name = name; }
	~WinRate() = default;

	double run()
	{
		if (m_strat.m_tradeCount <= 0)
		{
			m_Result = 0;
			return m_Result;
		}
		m_Result = (((double)m_strat.m_winTradeCount) / ((double)m_strat.m_tradeCount));
		return m_Result;
	};
public:
	Strategy& m_strat;
};

class MaxDD : public Analyzer
{
public:
	static constexpr const char* name = "MaxDD";
	MaxDD(Strategy& strat) : m_strat(strat) { m_name = name; }
	~MaxDD() = default;

	double run()
	{
		auto data = m_strat.m_equityCurve;

		size_t dataSize = data.size();
		if (dataSize == 0)
		{
			m_Result = 0;
			return m_Result;
		}

		double maxRet = data[0];
		double maxDD  = 0;

		for (size_t i = 0; i < dataSize; i++)
		{
			if (maxRet < data[i])
			{
				maxRet = data[i];
			}

			double DD = 0;
			if (maxRet != 0.0)
				DD = ((maxRet - data[i]) / maxRet) * 100.0;

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


class DrawDown : public Analyzer
{
public:
	static constexpr const char* name = "DrawDown";
	DrawDown(Strategy& strat) : m_strat(strat) { m_name = name; }
	~DrawDown() = default;

	double run()
	{
		auto& data = m_strat.m_equityCurve;
		m_line.clear();
		m_line.resize(data.size(), 0.0);

		size_t dataSize = data.size();
		if (dataSize == 0)
		{
			m_Result = 0;
			return m_Result;
		}

		double peak = data[0];
		double maxDD = 0;

		for (size_t i = 0; i < dataSize; i++)
		{
			if (data[i] > peak) peak = data[i];
			double dd = (peak > 0) ? (100.0 * (peak - data[i]) / peak) : 0.0;
			m_line[i] = dd;
			if (dd > maxDD) maxDD = dd;
		}
		m_Result = maxDD;
		return m_Result;
	}
public:
	Strategy& m_strat;
};



class SQN : public Analyzer
{
public:
	static constexpr const char* name = "SQN";
	SQN(Strategy& strat) : m_strat(strat) { m_name = name; }
	~SQN()= default;

	double run()
	{
		auto data = m_strat.m_PNL;
		size_t dataSize = data.size();
		const size_t startIdx = 10;
		if (dataSize <= startIdx)
		{
			m_Result = 0;
			return m_Result;
		}
		const size_t sampleSize = dataSize - startIdx;

		double sum  = 0.0;
		for (size_t i = startIdx; i < dataSize; i++)
		{
			sum += data[i];
		}

		double mean = sum / (double)sampleSize;

		double varianceSum = 0.0;
		for (size_t i = startIdx; i < dataSize; i++)
		{
			varianceSum += pow(data[i] - mean, 2);
		}
		double variance = varianceSum / (double)sampleSize;
		if (variance <= 0.0)
		{
			m_Result = 0;
			return m_Result;
		}

		m_Result = (mean * sqrt((double)sampleSize)) / sqrt(variance);
		return m_Result;
	};
public:
	Strategy& m_strat;
};


class ProfitFactor : public Analyzer
{
public:
	static constexpr const char* name = "ProfitFactor";
	ProfitFactor(Strategy& strat) : m_strat(strat) { m_name = name; }
	~ProfitFactor() = default;

	double run()
	{
		auto data = m_strat.m_PNL;
		size_t dataSize = data.size();
		if (dataSize == 0)
		{
			m_Result = 0;
			return m_Result;
		}

		double Psum = 0.0;
		double Nsum = 0.0;

		for (size_t i = 0; i < dataSize; i++)
		{
			if (data[i] > 0)
			{
				Psum += data[i];
			}
			else
			{
				Nsum += data[i];
			}
		}
		double denom = -Nsum;
		if (denom <= 0.0)
		{
			m_Result = 0;
			return m_Result;
		}
		m_Result = (Psum / denom);
		return m_Result;
	};
public:
	Strategy& m_strat;
};


class Expectancy : public Analyzer
{
public:
	static constexpr const char* name = "Expectancy";
	Expectancy(Strategy& strat) : m_strat(strat) { m_name = name; }
	~Expectancy() = default;

	double run()
	{
		const int n = m_strat.m_tradeCount;
		if (n <= 0) { m_Result = 0; return m_Result; }

		const auto& pnl = m_strat.m_PNL;
		if ((size_t)n != pnl.size()) { m_Result = 0; return m_Result; }

		double sumWins = 0.0;
		double sumLosses = 0.0;
		int winCount = 0;
		int lossCount = 0;

		for (int i = 0; i < n; i++)
		{
			if (pnl[i] > 0)
			{
				sumWins += pnl[i];
				winCount++;
			}
			else
			{
				sumLosses += pnl[i]; // negative
				lossCount++;
			}
		}

		double avgWin = (winCount > 0) ? (sumWins / winCount) : 0.0;
		double avgLoss = (lossCount > 0) ? (sumLosses / lossCount) : 0.0;
		double winRate = (double)winCount / n;
		double lossRate = (double)lossCount / n;

		m_Result = (winRate * avgWin) + (lossRate * avgLoss);
		return m_Result;
	}
public:
	Strategy& m_strat;
};


class AvgDD : public Analyzer
{
public:
	static constexpr const char* name = "AvgDD";
	AvgDD(Strategy& strat) : m_strat(strat) { m_name = name; }
	~AvgDD() = default;

	double run()
	{
		auto& data = m_strat.m_equityCurve;
		size_t dataSize = data.size();
		if (dataSize == 0) { m_Result = 0; return m_Result; }

		double peak = data[0];
		double sumDD = 0.0;
		size_t count = 0;

		for (size_t i = 0; i < dataSize; i++)
		{
			if (data[i] > peak) peak = data[i];
			double dd = (peak > 0) ? (100.0 * (peak - data[i]) / peak) : 0.0;
			sumDD += dd;
			count++;
		}

		m_Result = (count > 0) ? (sumDD / count) : 0.0;
		return m_Result;
	}
public:
	Strategy& m_strat;
};


class SharpeRatio : public Analyzer
{
public:
	static constexpr const char* name = "SharpeRatio";
	SharpeRatio(Strategy& strat) : m_strat(strat) { m_name = name; }
	~SharpeRatio() = default;

	double run()
	{
		const auto& trades = m_strat.m_trades;
		size_t n = trades.size();
		if (n < 2)
		{
			m_Result = 0;
			return m_Result;
		}

		double sum = 0.0;
		for (size_t i = 0; i < n; i++)
			sum += trades[i].second;

		double mean = sum / static_cast<double>(n);

		double varSum = 0.0;
		for (size_t i = 0; i < n; i++)
			varSum += (trades[i].second - mean) * (trades[i].second - mean);
		double std = std::sqrt(varSum / static_cast<double>(n - 1));

		if (std <= 1e-12)
		{
			m_Result = 0;
			return m_Result;
		}
		const double baseSharpe = mean / std;
		const size_t usedCandles = (m_strat.m_Data && m_strat.m_Data->close.size() > 1)
			? (m_strat.m_Data->close.size() - 1)
			: 0;
		const int candlesPerDay = OHLC::candlesPerDay(m_strat.m_Data ? m_strat.m_Data->m_CandleType : OHLC::m15);
		const double yearsCovered = (candlesPerDay > 0)
			? (static_cast<double>(usedCandles) / (static_cast<double>(candlesPerDay) * 365.0))
			: 0.0;

		// User-requested scaling: multiply Sharpe by tested year span.
		m_Result = baseSharpe * yearsCovered;
		return m_Result;
	}
public:
	Strategy& m_strat;
};
