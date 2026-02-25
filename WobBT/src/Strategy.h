#pragma once
#include <vector>
#include <utility>
#include <stdio.h>
#include <iostream>
#include <map>
#include <string>
#include <algorithm>
#include "OHLC.h"
#include "Debug.h"
#include "Indicator.h"
#include "Analyzer.h"
#include "IBroker.h"
#include "matplotlib/matplotlibcpp.h"
namespace plt = matplotlibcpp;

// Generic Strategy Class and virtual functions
// Takes a param array of integers as params

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

	void Plot(std::vector<double>* equityCurve, bool plotIndicators = true)
	{
		if (!equityCurve || equityCurve->empty()) return;

		size_t n = equityCurve->size();

		// Get drawdown from Cerebro DrawDown analyzer (only if present)
		std::vector<double> drawdown;
		for (auto* a : m_Analyzers)
		{
			if (a->m_name == "DrawDown" && a->m_line.size() == n)
			{
				drawdown = a->m_line;
				break;
			}
		}

		// X-axis indices for plots (1-based to match candle indexing)
		std::vector<double> xidx(n);
		for (size_t i = 0; i < n; i++) xidx[i] = static_cast<double>(i);

		plt::figure_size(1400, 1000);

		std::vector<Indicator*> plotInds;
		std::vector<Indicator*> subplotInds;
		for (size_t i = 0; i < m_Inds.size(); i++)
		{
			if (m_Inds[i]->isSubplot)
				subplotInds.push_back(m_Inds[i]);
			else
				plotInds.push_back(m_Inds[i]);
		}

		// Grid: broker(1) + trades(1) + drawdown(1 if present) + main(4) + indicators(N if plotIndicators), main has 4x weight
		const int MAIN_WEIGHT = 4;
		int indCount = plotIndicators ? static_cast<int>(subplotInds.size()) : 0;
		int smallRows = (m_cashCurve.size() == n ? 2 : 1) + (!drawdown.empty() ? 1 : 0) + indCount;
		int gridRows = smallRows + MAIN_WEIGHT;
		double xMin = 0, xMax = static_cast<double>(n - 1);

		std::map<std::string, double> m{ {"left", 0.06}, {"right", 0.96}, {"bottom", 0.04}, {"top", 0.96}, {"hspace", 0.25}, {"wspace", 0.2} };
		plt::subplots_adjust(m);

		int row = 0;

		auto addLabelTopLeft = [&](double yMin, double yMax, const std::string& label) {
			double yPos = yMax - 0.03 * (yMax - yMin);
			if (yMax <= yMin) yPos = yMin;
			plt::text(xMin, yPos, label);
		};

		// 1. Broker: cash + value (if we have cash curve)
		if (m_cashCurve.size() == n)
		{
			plt::subplot2grid(gridRows, 1, row++, 0, 1, 1);
			plt::named_plot("cash", xidx, m_cashCurve, "r-");
			plt::named_plot("value", xidx, *equityCurve, "b-");
			double yMin = std::min(*std::min_element(m_cashCurve.begin(), m_cashCurve.end()), *std::min_element(equityCurve->begin(), equityCurve->end()));
			double yMax = std::max(*std::max_element(m_cashCurve.begin(), m_cashCurve.end()), *std::max_element(equityCurve->begin(), equityCurve->end()));
			addLabelTopLeft(yMin, yMax, "Broker");
			plt::ylabel("Cash / Value");
			plt::legend();
			plt::grid(true);
			plt::xlim(xMin, xMax);
		}

		// 2. Trades - PNL scatter (positive blue, negative red)
		plt::subplot2grid(gridRows, 1, row++, 0, 1, 1);
		std::vector<double> tradeXPos, tradeYPos, tradeXNeg, tradeYNeg;
		for (const auto& t : m_trades)
		{
			double x = static_cast<double>(t.first - 1);
			if (t.first >= 1 && t.first <= (int)n)
			{
				if (t.second >= 0) { tradeXPos.push_back(x); tradeYPos.push_back(t.second); }
				else { tradeXNeg.push_back(x); tradeYNeg.push_back(t.second); }
			}
		}
		if (!tradeXPos.empty())
			plt::scatter(tradeXPos, tradeYPos, 30, { {"c", "blue"}, {"label", "Positive"} });
		if (!tradeXNeg.empty())
			plt::scatter(tradeXNeg, tradeYNeg, 30, { {"c", "red"}, {"label", "Negative"} });
		plt::axhline(0.0, 0.0, 1.0, { {"color","k"}, {"linestyle","--"} });
		double tradeYMin = -1, tradeYMax = 1;
		if (!tradeYPos.empty() || !tradeYNeg.empty())
		{
			std::vector<double> allY; for (auto y : tradeYPos) allY.push_back(y); for (auto y : tradeYNeg) allY.push_back(y);
			tradeYMin = *std::min_element(allY.begin(), allY.end()); tradeYMax = *std::max_element(allY.begin(), allY.end());
		}
		addLabelTopLeft(tradeYMin, tradeYMax, "Trades - Net P/L %");
		plt::ylabel("PnL %");
		plt::grid(true);
		plt::xlim(xMin, xMax);

		// 3. DrawDown (only if analyzer provided data)
		if (!drawdown.empty())
		{
			plt::subplot2grid(gridRows, 1, row++, 0, 1, 1);
			plt::plot(xidx, drawdown, "r-");
			double ddMin = 0, ddMax = *std::max_element(drawdown.begin(), drawdown.end());
			if (ddMax < 0.01) ddMax = 1;
			addLabelTopLeft(ddMin, ddMax, "DrawDown");
			plt::ylabel("%");
			plt::grid(true);
			plt::xlim(xMin, xMax);
		}

		// 4. Main price chart: volume (back), close, overlay indicators, buy/sell signals (4x height)
		plt::subplot2grid(gridRows, 1, row, 0, MAIN_WEIGHT, 1);
		row += MAIN_WEIGHT;

		std::vector<double> closePlot(m_Data->close.begin() + 1, m_Data->close.end());
		if (closePlot.size() > n) closePlot.resize(n);

		// Volume first (background)
		if (m_Data->volume.size() > 1)
		{
			size_t volLen = std::min(m_Data->volume.size() - 1, n);
			if (volLen > 0)
			{
				double volMax = 0;
				for (size_t i = 1; i <= volLen && i < m_Data->volume.size(); i++)
					if (m_Data->volume[i] > volMax) volMax = m_Data->volume[i];
				double priceMin = *std::min_element(closePlot.begin(), closePlot.end());
				double priceRange = (*std::max_element(closePlot.begin(), closePlot.end())) - priceMin;
				if (priceRange < 1e-9) priceRange = 1;
				if (volMax > 0)
				{
					std::vector<double> volX(volLen), volBase(volLen), volTop(volLen);
					for (size_t i = 0; i < volLen; i++)
					{
						volX[i] = static_cast<double>(i);
						volBase[i] = priceMin;
						volTop[i] = priceMin + (m_Data->volume[i + 1] / volMax) * priceRange * 0.12;
					}
					plt::fill_between(volX, volBase, volTop, { {"color", "0.8"} });
				}
			}
		}

		// Price line
		plt::plot(closePlot, "k-");

		// Overlay indicators on price (if plotIndicators)
		if (plotIndicators)
		{
			const char* formats[] = { "g-", "r-", "m-", "c-", "y-" };
			for (size_t i = 0; i < plotInds.size(); i++)
			{
				auto& line = plotInds[i]->line;
				if (line.size() > n)
					plt::plot(std::vector<double>(line.begin() + 1, line.begin() + 1 + n), formats[i % 5]);
			}
		}

		double priceMin = *std::min_element(closePlot.begin(), closePlot.end());
		double priceMax = *std::max_element(closePlot.begin(), closePlot.end());
		double priceRange = priceMax - priceMin;
		if (priceRange < 1e-9) priceRange = 1;
		double signalOffset = priceRange * 0.002;

		// Buy signals (green triangles) - slightly below price
		std::vector<double> buyX, buyY;
		for (const auto& s : m_buySignals)
		{
			if (s.first >= 1 && s.first <= (int)n)
			{
				buyX.push_back(static_cast<double>(s.first - 1));
				buyY.push_back(s.second - signalOffset);
			}
		}
		if (!buyX.empty())
			plt::scatter(buyX, buyY, 80, { {"c", "green"}, {"marker", "^"} });

		// Sell signals (red triangles) - slightly above price
		std::vector<double> sellX, sellY;
		for (const auto& s : m_sellSignals)
		{
			if (s.first >= 1 && s.first <= (int)n)
			{
				sellX.push_back(static_cast<double>(s.first - 1));
				sellY.push_back(s.second + signalOffset);
			}
		}
		if (!sellX.empty())
			plt::scatter(sellX, sellY, 80, { {"c", "red"}, {"marker", "v"} });
		addLabelTopLeft(priceMin, priceMax, "Price");
		plt::ylabel("Price / Volume");
		plt::grid(true);
		plt::xlim(xMin, xMax);

		// Subplot indicators (if plotIndicators)
		for (size_t i = 0; i < (plotIndicators ? subplotInds.size() : 0); i++)
		{
			plt::subplot2grid(gridRows, 1, row++, 0, 1, 1);
			auto& line = subplotInds[i]->line;
			std::vector<double> indLine;
			if (line.size() > n)
				indLine.assign(line.begin() + 1, line.begin() + 1 + n);
			else if (!line.empty())
				indLine.assign(line.begin(), line.begin() + std::min(line.size(), n));
			if (!indLine.empty())
			{
				plt::plot(indLine, "r-");
			}
			double indMin = 0, indMax = 1;
			if (!indLine.empty())
			{
				indMin = *std::min_element(indLine.begin(), indLine.end());
				indMax = *std::max_element(indLine.begin(), indLine.end());
				if (indMax <= indMin) indMax = indMin + 1;
			}
			addLabelTopLeft(indMin, indMax, subplotInds[i]->m_name);
			plt::ylabel("Value");
			plt::grid(true);
			plt::xlim(xMin, xMax);
		}

		plt::xlabel("Candle Index");
		plt::show();
	}

	void printResults()
	{
		Debug::Log("------------------------------------------------------------------------------------------");
		Debug::Log("------------------------------------ RESULTS ---------------------------------------------");
		for (auto* var : m_Inds)
		{
			auto len = var->line.size();
			Debug::Log(var->m_name + ": " + std::to_string(var->line[len - 1]));
		}
		Debug::Log("------------------------------------------------------------------------------------------");

		for (auto* var : m_Analyzers)
		{
			var->printResult();
		}
		Debug::Log("------------------------------------------------------------------------------------------");
	}

	void deleteElements()
	{
		for (auto* var : m_Inds)
			delete var;
		m_Inds.clear();

		for (auto* var : m_Analyzers)
			delete var;
		m_Analyzers.clear();
	}

	double end(double close)
	{ 
		return getEQ(close);
	};

	void Orderer(int candleIndex, bool isbuy, std::string reason)
	{
		if (m_isOrdered)
			return;
		if (m_warmupMode)
			return;

		double close = m_Data->close[candleIndex];
		if (isbuy)
		{
			if (m_buyprice == -1)
			{
				m_isOrdered = true;
				m_buyprice  = close;
				m_buysize   = m_Cash / m_buyprice;
				m_buysize  -= m_buysize * m_commissions;
				m_Cash      = 0;

				m_buySignals.push_back({ candleIndex, close });

				if (m_liveBroker)
					m_liveBroker->executeOrder(true, close, m_buysize);
			}
		}
		else
		{
			if (m_buyprice != -1)
			{
				m_isOrdered = true;

				if (m_liveBroker)
					m_liveBroker->executeOrder(false, close, m_buysize);

				m_Cash = close * m_buysize;
				m_Cash -= m_Cash * m_commissions;
				double pnl = (m_Cash) - (m_buyprice * m_buysize);
				m_PNL.push_back(pnl);
				double costBasis = m_buyprice * m_buysize;
				double pnlPct = (costBasis != 0) ? (100.0 * pnl / costBasis) : 0.0;
				m_trades.push_back({ candleIndex, pnlPct });

				m_sellSignals.push_back({ candleIndex, close });

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
			}
		}
	}

	double run()
	{
		for (size_t i = 1; i < m_Data->close.size(); i++)
		{
			m_isOrdered = false;
			next(i);
			double eq = getEQ(m_Data->close[i]);
			m_equityCurve.push_back(eq);
			m_cashCurve.push_back(m_Cash);
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
	std::vector<double> m_equityCurve;
	std::vector<double> m_cashCurve;
	std::vector<double> m_PNL;
	std::vector<std::pair<int, double>> m_trades;       // (candleIndex, pnl_pct)
	std::vector<std::pair<int, double>> m_buySignals;   // (candleIndex, price)
	std::vector<std::pair<int, double>> m_sellSignals;  // (candleIndex, price)

public:
	double m_Cash        = 1000;
	double m_buyprice    = -1;
	double m_buysize     =  0;
	double m_commissions = 0.001;

	// Live trading: broker executes orders; warmup skips Orderer
	IBroker* m_liveBroker = nullptr;
	bool     m_warmupMode = false;
public:
	int    m_init_all_ind_periods = 0;
	int    m_tradeCount = 0;
	int    m_winTradeCount = 0;
	int    m_winStreak = 0;
	int    m_loseStreak = 0;
	int    m_max_winStreak = 0;
	int    m_max_loseStreak = 0;

	bool   m_isOrdered = false;
};