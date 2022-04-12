#include "multithread.hpp"

unsigned int threadsID = 0;
volatile unsigned long long Massive_Number = 0;

std::mutex SafeLock{};

int main()
{
	/*
		스레드 실행 시간이 오락가락한다.
	*/
	std::chrono::high_resolution_clock clock{};
	auto time_point = clock.now();

	SingleAddWorker();

	//while (true); // join을 미리 하면 main만 CPU를 사용한다.
	auto later_point = clock.now();
	auto period = std::chrono::duration_cast<std::chrono::milliseconds>(later_point - time_point);
	auto now_point = period.count();

	std::cout << "싱글 스레드 걸린 시간 (밀리초): " << now_point << ".\n";
	std::cout << "결과 값: " << Massive_Number << "\n";

	Massive_Number = 0;
	std::vector<std::thread> threads{ 4 };
	time_point = clock.now();

	threads.emplace_back(SafeAddWorker);
	threads.emplace_back(SafeAddWorker);
	threads.emplace_back(SafeAddWorker);
	threads.emplace_back(SafeAddWorker);

	std::for_each(threads.begin(), threads.end(), [](std::thread& th) {
		if (th.joinable())
			th.join();
	});

	later_point = clock.now();
	period = std::chrono::duration_cast<std::chrono::milliseconds>(later_point - time_point);
	now_point = period.count();

	std::cout << "멀티스레드 걸린 시간 (밀리초): " << now_point << ".\n";
	std::cout << "결과 값: " << Massive_Number << "\n";

	//worker.join(); // 없으면 Worker의 작동이 중간에 중단된다.

	return 0;
}

void Worker(unsigned int id)
{
	// std::this_thread::get_id()
	std::cout << "I'm a thread! " << id << "\n";
}

void SingleAddWorker()
{
	for (int i = 0; i < 50000000; ++i)
	{
		Massive_Number += 2;
	}
}

void MassiveAddWorker()
{
	for (int i = 0; i < 12500000; ++i)
	{
		Massive_Number += 2;
	}
}

void SafeAddWorker()
{
	SafeLock.lock();
	MassiveAddWorker();
	SafeLock.unlock();
}

