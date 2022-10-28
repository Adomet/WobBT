#pragma once
#include <vector>
#include <stdio.h>
#include <iostream>
#include <map>
#include "ta_libc.h"
#include "OHLC.h"
#include "Debug.h"
#include "Indicator.h"
#include "Analyzer.h"
#include "matplotlib/matplotlibcpp.h"
namespace plt = matplotlibcpp;

// Generic Strategy Class and virtual functions
// Takes a param array array of intager as params


class Strategy
{
public:
	Strategy(OHLC* data,std::vector<int> params):m_Data(data),m_params(params) {};
	Strategy(double startCash, double commissions):m_Cash(startCash),m_commissions(commissions){};
	~Strategy(){};

	virtual void init(){};
	virtual void next(int candleIndex) {};

	double getEQ(double close)
	{
		return m_Cash + (m_buysize * close);
	}

	void Plot()
	{
		// Prepare data
		// int n = 500;
		// std::vector<double> x(n), y(n), z(n), w(n, 2);
		// for (int i = 0; i < n; ++i) {
		// 	x.at(i) = i;
		// 	y.at(i) = sin(2 * 3.14 * i / 360.0);
		// 	z.at(i) = 100.0 / i;
		// }
		// 
		// // Set the "super title"
		// plt::suptitle("My plot");
		// plt::subplot(1, 2, 1);
		// plt::plot(x, y, "r-");
		// plt::subplot(1, 2, 2);
		// plt::plot(x, z, "k-");
		// // Add some text to the plot
		// plt::text(100, 90, "Hello!");
		// 
		// 
		// // Show plots
		// plt::show();

		std::map<std::string, double> m{ {"left", 0.03}, {"right", 0.97},{"bottom", 0.03} ,{"top", 0.97}};

		plt::subplots_adjust(m);
		
		int c = 1;
		

		for each (auto var in m_Inds)
		{
			if (!var->isSubplot)
			{
				plt::plot(var->line);

				auto len = var->line.size();
				plt::text(0, (int)var->line[0], var->m_name);

				auto tmp = var->line[len - 1];
				plt::text((int)len, (int)tmp, std::to_string(tmp));
			}
		}

		size_t size = m_Inds.size() - c-1;
		for each (auto var in m_Inds)
		{
			if (var->isSubplot)
			{
				plt::subplot(size, 1, c++);
				plt::plot(var->line);
		
		
				auto len = var->line.size();
				plt::text(0, (int)var->line[0], var->m_name);
		
				auto tmp = var->line[len - 1];
				plt::text((int)len, 0, std::to_string(tmp));
			}
		}

		plt::show();
	}


	void printResults()
	{
		Debug::Log("------------------------------------------------------------------------------------------");
		Debug::Log("------------------------------------ RESULTS ---------------------------------------------");
		for each (auto var in m_Inds)
		{
			auto len = var->line.size();
			Debug::Log(var->m_name +": " + std::to_string(var->line[len - 1]));
		}
		Debug::Log("------------------------------------------------------------------------------------------");

		for each (auto var in m_Analyzers)
		{
			var->printResult();
		}
		Debug::Log("------------------------------------------------------------------------------------------");

	}


	void deleteElements()
	{
		for each (auto var in m_Inds)
		{
			delete var;
		}

		for each (auto var in m_Analyzers)
		{
			delete var;
		}
	}

	double end(double close)
	{ 

		return getEQ(close);
	};

	void Orderer(int candleIndex, bool isbuy,std::string reason)
	{
		double ret = 0;
		double close = m_Data->open[candleIndex+1];
		if (isbuy)
		{
			if (m_buyprice == -1)
			{
				m_buyprice  = close;
				m_buysize   = m_Cash / m_buyprice;
				m_buysize  -= m_buysize * m_commissions;
				m_Cash      = 0;
				//Debug::Log(reason +" B " + std::to_string(close));

			}
			else
			{
				//Debug::Log("Trying to buy but already in Pos");
			}
		}
		else
		{
			if (m_buyprice != -1)
			{
				m_Cash = close * m_buysize;
				m_Cash -= m_Cash * m_commissions;
				m_PNL.push_back((m_Cash)-(m_buyprice * m_buysize));

				if (m_buyprice < close)
				{
					m_winTradeCount++;
					m_winStreak++;
					m_loseStreak = 0;
					if (m_max_winStreak < m_winStreak)
						m_max_winStreak = m_winStreak;
				}
				else
				{
					m_loseStreak++;
					m_winStreak = 0;
					if (m_max_loseStreak < m_loseStreak)
						m_max_loseStreak = m_loseStreak;
				}

				m_buyprice = -1;
				m_buysize  =  0;
				m_tradeCount++;
				//Debug::Log(reason + " S " + std::to_string(close));
			}
			else
			{
				//Debug::Log("Trying to sell but NOT in Pos");
			}
		}
	}

	double  run()
	{


		double ret = 0;
		for (size_t i = 0; i < m_Data->close.size(); i++)
		{
			next(i);
			m_eqQurve.push_back(getEQ(m_Data->open[i+1]));
		}

		for (size_t i = 0; i < m_Analyzers.size(); i++)
		{
			m_Analyzers[i]->run();

		}
		

		return end(m_Data->close[m_Data->close.size() - 1]);
	}


public:
	OHLC* m_Data = nullptr;
	std::vector<int> m_params;
	std::vector<Indicator*> m_Inds;
	std::vector<Analyzer*>  m_Analyzers;
	std::vector<double> m_eqQurve;
	std::vector<double> m_PNL;

public:
	double m_Cash        = 1000;
	double m_buyprice    = -1;
	double m_buysize     =  0;
	double m_commissions = 0.001;
public:
	int    m_init_all_ind_periods = 0;
	int    m_tradeCount = 0;
	int    m_winTradeCount = 0;
	int    m_winStreak = 0;
	int    m_loseStreak = 0;
	int    m_max_winStreak = 0;
	int    m_max_loseStreak = 0;
};