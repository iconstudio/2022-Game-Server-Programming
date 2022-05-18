#include "stdafx.hpp"
#include "Framework.hpp"

IOCPFramework framework{};

int main()
{
	framework.Init();
	framework.Start();

	return 0;
}

void CALLBACK IOCPWorker()
{
	while (true)
	{
		framework.Update();
	}
}
