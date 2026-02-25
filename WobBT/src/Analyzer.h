#pragma once
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include "Debug.h"
class Analyzer
{
public:
	Analyzer() = default;
	virtual ~Analyzer() = default;

	virtual double run() { return 0; };

	void printResult() { Debug::Log(m_name + ": " + std::to_string(m_Result)); };

public:
	double m_Result = 0;
	std::string m_name = "Analyzer";
	std::vector<double> m_line;  // time-series result (e.g. drawdown curve)
};