#pragma once
#include "IBroker.h"

class Broker : public IBroker
{
public:
	explicit Broker(double startCash = 1000, double commissions = 0.001);

	void executeOrder(bool isBuy, double price) override;
	BrokerState getState() override;
	void setStartCash(double startCash) override;
	void setCommissions(double commissions) override;
	double getCommissions() const override { return m_commissions; }

private:
	BrokerState m_state;
	double m_commissions = 0.001;
};
