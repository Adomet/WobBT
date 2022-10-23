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
	void setStartCash(float startCash) { m_Strategy->m_Cash = startCash; }
	void setCommissons(float commission) { m_Strategy->m_commissions = commission; }
	float run();

private:
	Strategy* m_Strategy;
};