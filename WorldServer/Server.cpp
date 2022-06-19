#include "pch.hpp"
#include "Framework.hpp"

IOCPFramework framework{};

int main()
{
	framework.Awake();
	framework.Start();
	framework.Update();
	framework.Release();

	return 0;
}

void IOCPWorker()
{
	while (true)
	{
		framework.Communicate();
	}
}

void AIWorker()
{

}

void TimerWorker()
{

}
