#include "Cerebro.h"

float Cerebro::run()
{
	// Calculate Return based on buy and sell locations - commisons
	float ret = 0;
	m_Strategy->init();
	ret = m_Strategy->run();
	//Create a Analyzed class to return 
	return ret;
}


