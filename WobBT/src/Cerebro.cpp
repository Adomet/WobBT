#include "Cerebro.h"
#include "Debug.h"

double CerebroResult::getResult(const std::string& name) const
{
	auto ait = analyzers.find(name);
	if (ait != analyzers.end())
		return ait->second;
	auto iit = indicators.find(name);
	if (iit != indicators.end())
		return iit->second;
	return 0.0;
}

void CerebroResult::print() const
{
	Debug::Log("------------------------------------------------------------------------------------------");
	Debug::Log("------------------------------------ RESULTS ---------------------------------------------");
	//for (const auto& p : indicators)
	//	Debug::Log(p.first + ": " + std::to_string(p.second));
	for (const auto& p : analyzers)
		Debug::Log(p.first + ": " + std::to_string(p.second));
	Debug::Log("------------------------------------------------------------------------------------------");
}

CerebroResult Cerebro::run()
{
	CerebroResult result;
	m_Strategy->init();
	m_Strategy->run();

	for (Analyzer* a : m_Strategy->m_Analyzers)
		result.analyzers[a->m_name] = a->m_Result;

	for (Indicator* ind : m_Strategy->m_Inds)
	{
		if (!ind->line.empty())
			result.indicators[ind->m_name] = ind->line.back();
	}

	return result;
}


