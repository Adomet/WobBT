#pragma once
#include <stdio.h>
#include <iostream>
#include <memory>
#include <chrono>

#define DEBUG_MODE 

class Debug
{
public:
	Debug(){};
	~Debug(){};

	static void Log(std::string log)
	{
#ifdef DEBUG_MODE
		std::cout << log << std::endl;
#endif // DEBUG_MODE
	}

	static void Log(const char* log)
	{
#ifdef DEBUG_MODE
		std::cout << log << std::endl;
#endif // DEBUG_MODE
	}

private:

};


class Timer
{
public:
	Timer(std::string jobName)
	{
		m_jobName = jobName;
		m_StartTimePoint = std::chrono::high_resolution_clock::now();
	}
	~Timer()
	{
		Stop();
	}

	void Stop()
	{
		auto endTimePoint = std::chrono::high_resolution_clock::now();

		auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimePoint).time_since_epoch().count();
		auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimePoint).time_since_epoch().count();

		auto duration = end - start;
		double ms = duration * 0.001;
		double s = ms * 0.001;


		//std::cout << s << "s " << duration << "us (" << ms << "ms)\n" << std::endl;
		std::cout << m_jobName << " Took: " << s << "s " << std::endl;
	}

private:
	std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimePoint;
	std::string m_jobName;
};
