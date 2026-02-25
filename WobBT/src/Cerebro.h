#pragma once
#include "Strategy.h"
#include <map>
#include <string>

// Cerebro is brain of system
// Gets: Data, Strat info about broker(commissions etc.)
// Returns aggregated analyzer and indicator results

struct CerebroResult
{
	std::map<std::string, double> analyzers;

	// Last value of each indicator (indicator name -> last line value)
	std::map<std::string, double> indicators;

	// Optimization context (set by caller when used in param scan)
	int scan_param = -1;
	double score = 0;

	double getResult(const std::string& name) const;

	template <typename T>
	double getResult() const { return getResult(T::name); }

	void print() const;
};

class Cerebro
{
public:
	Cerebro(Strategy* strat) : m_Strategy(strat) { setStartCash(); setCommissions(); }
	~Cerebro() = default;

	void setStartCash(double startCash = 1000) { m_Strategy->m_Cash = startCash; }
	void setCommissions(double commission = 0.001) { m_Strategy->m_commissions = commission; }
	void addAnalyzer(Analyzer* analyzer) { m_Strategy->m_Analyzers.push_back(analyzer); }

	Strategy* getStrategy() { return m_Strategy; }

	CerebroResult run();

private:
	Strategy* m_Strategy;
};