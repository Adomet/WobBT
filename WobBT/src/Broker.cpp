#include "Broker.h"

Broker::Broker(double startCash, double commissions)
	: m_commissions(commissions)
{
	m_state.cash = startCash;
	m_state.coin = 0;
	m_state.lastBuyPrice = -1;
}

void Broker::executeOrder(bool isBuy, double price)
{
	if (isBuy)
	{
		double qty = m_state.cash / price;
		qty -= qty * m_commissions;
		m_state.coin = qty;
		m_state.cash = 0;
		m_state.lastBuyPrice = price;
	}
	else
	{
		m_state.cash = m_state.coin * price;
		m_state.cash -= m_state.cash * m_commissions;
		m_state.coin = 0;
		m_state.lastBuyPrice = -1;
	}
}

BrokerState Broker::getState()
{
	return m_state;
}

void Broker::setStartCash(double startCash)
{
	m_state.cash = startCash;
	m_state.coin = 0;
	m_state.lastBuyPrice = -1;
}

void Broker::setCommissions(double commissions)
{
	m_commissions = commissions;
}
