#pragma once

struct BrokerState
{
	double cash = 0;
	double coin = 0;
	double lastBuyPrice = -1;

	bool inPosition() const { return lastBuyPrice > 0; }
};

class IBroker
{
public:
	virtual void executeOrder(bool isBuy, double price) = 0;
	virtual BrokerState getState() = 0;
	virtual void setStartCash(double) {}
	virtual void setCommissions(double) {}
	virtual double getCommissions() const { return 0; }
	virtual ~IBroker() = default;
};
