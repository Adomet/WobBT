#pragma once
#include <vector>
#include <stdio.h>
#include <iostream>
#include <map>
#include "ta_libc.h"
#include "OHLC.h"
#include "Debug.h"
#include "Indicator.h"
// Generic Strategy Class and virtual functions
// Takes a param array array of intager as params

class Strategy
{
public:
	Strategy(OHLC* data,std::vector<int> params):m_Data(data),m_params(params) {};
	Strategy(float startCash, float commissions):m_Cash(startCash),m_commissions(commissions){};
	~Strategy(){};

	virtual void init(){};
	virtual void next(int candleIndex) {};

	float end(float close) 
	{ 
		for each (auto var in m_Inds)
		{
			delete var;
		}

		return m_Cash + (m_holdings * close);
	};

	void Orderer(int candleIndex, bool isbuy)
	{
		float ret = 0;
		float close = m_Data->close[candleIndex];
		if (isbuy)
		{
			if (m_buyPos == -1)
			{
				m_buyPos    = close;
				m_holdings  = m_Cash / m_buyPos;
				m_holdings -= m_holdings * m_commissions;
				m_Cash      = 0;

				//Debug::Log("Buyed at:" +std::to_string(close));

			}
			else
			{
				//Debug::Log("Trying to buy but already in Pos");
			}
		}
		else
		{
			if (m_buyPos != -1)
			{
				m_Cash = close * m_holdings;
				m_Cash -= m_Cash * m_commissions;

				m_buyPos   = -1;
				m_holdings =  0;

				//Debug::Log("Selled at:" + std::to_string(close));
			}
			else
			{
				//Debug::Log("Trying to sell but NOT in Pos");
			}
		}
	}

	float  run()
	{
		float ret = 0;
		for (size_t i = 0; i < m_Data->close.size(); i++)
		{
			next(i);
			if (i == m_Data->close.size() - 1)
			{
				ret = end(m_Data->close[i]);
			}
		}

		return ret;
	}


public:
	OHLC* m_Data = nullptr;
	std::vector<int> m_params;
	std::vector<Indicator*> m_Inds;
	float m_Cash = 1000;
	float m_buyPos   = -1;
	float m_holdings =  0;
	float m_commissions = 0.001;
	int   m_init_all_ind_periods = 0;
};