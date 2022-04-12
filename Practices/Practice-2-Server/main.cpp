#include "pch.hpp"
#include "main.hpp"

#include "Network.hpp"

//NetworkServer server;

std::mutex Mutex;

constexpr UINT NumberThread = 8;
volatile int sum = 0;
struct Summary
{
	Summary(const int nv) : value(nv) {}

	volatile int value = 0;
};

Summary summaries[NumberThread];

void Worker(const UINT thread_id)
{
	volatile int local_sum = 0;
	for (int i = 0; i < 100000000; ++i)
	{
		local_sum += 2;
	}

	Mutex.lock();
	sum += local_sum;
	Mutex.unlock();
}

void AlignedArrayWorker(const UINT thread_id)
{
	volatile int local_sum = 0;
	for (int i = 0; i < 100000000; ++i)
	{
		local_sum += 2;
	}

	Mutex.lock();
	(summaries[thread_id]).value += local_sum;
	Mutex.unlock();
}

int main()
{
	std::vector<std::thread> dthreads(NumberThread);

	for (int k = 0; k < 8; ++k)
	{
		for (int i = 0; i < 100000000; ++i)
		{
			dthreads.emplace_back(Worker, k);
		}
		std::cout << k << ": " << sum << "\n";
	}

	//server.Start();

	return 0;
}
