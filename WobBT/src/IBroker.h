#pragma once

// Broker interface for live execution (Binance implements this)
class IBroker
{
public:
    virtual void executeOrder(bool isBuy, double price, double quantity) = 0;
    virtual ~IBroker() = default;
};
