#include "stdafx.hpp"
#include "Framework.hpp"

IOCPFramework framework{};

int main()
{
	framework.Init();
	framework.Start();

	return 0;
}

void IOCPWorker()
{
	while (true)
	{
		framework.Communicate();
	}
}
