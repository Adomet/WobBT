#pragma once
#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Image.h"

class WobBTApp
{
public:
	WobBTApp(int argc, char** argv);
	~WobBTApp();

	void Run();
private:
	Walnut::Application* m_App;

};
