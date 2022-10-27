#pragma once
#include "Strategy.h"

// Cerebro is brain of system
// Gets: Data, Strat info about broker(commissions etc.)
// returns analyzed info

class Cerebro
{
public:
	Cerebro::Cerebro(Strategy* strat) :m_Strategy(strat) {}
	~Cerebro() {};
	void setStartCash(double startCash) { m_Strategy->m_Cash = startCash; }
	void setCommissons(double commission) { m_Strategy->m_commissions = commission; }
	void addAnalyzer(Analyzer* analyzer) { m_Strategy->m_Analyzers.push_back(analyzer); }
	double run();

private:
	Strategy* m_Strategy;
};