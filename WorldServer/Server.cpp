#include "pch.hpp"
#include "Framework.hpp"
#include "Timeline.hpp"

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
	auto& queue = framework.timerQueue;
	auto& mutex = framework.timerMutex;
	const auto& port = framework.completionPort;

	using namespace std::chrono_literals;

	while (true)
	{
		mutex.lock();
		while (0 < queue.size())
		{
			auto& timed_event = queue.top();

			const auto now = std::chrono::system_clock::now();

			if (timed_event.act_time < now)
			{
				const auto& id = timed_event.object_id;
				const auto& target_id = timed_event.target_id; // option
				const auto& ops = timed_event.ev;

				auto asyncer = new Asynchron(ops);

				PostQueuedCompletionStatus(port, DWORD(target_id), ULONG_PTR(id), asyncer);

				queue.pop();
			}
			else
			{
				break;
			}
		}
		mutex.unlock();

		std::this_thread::sleep_for(10ms);
	}
}
