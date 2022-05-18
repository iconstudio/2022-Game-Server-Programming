#include "stdafx.hpp"
#include "Framework.hpp"

IOCPFramework framework{};

int main()
{
	framework.Init();
	framework.Start();

	return 0;
}

void IOCPWorker(const UINT index)
{
	while (true)
	{
		framework.Update();
	}
}
