#include "multithread.hpp"

unsigned int threadsID = 0;
volatile unsigned long long Massive_Number = 0;

std::mutex SafeLock{};

int main()
{
	/*
		������ ���� �ð��� ���������Ѵ�.
	*/
	std::chrono::high_resolution_clock clock{};
	auto time_point = clock.now();

	SingleAddWorker();

	//while (true); // join�� �̸� �ϸ� main�� CPU�� ����Ѵ�.
	auto later_point = clock.now();
	auto period = std::chrono::duration_cast<std::chrono::milliseconds>(later_point - time_point);
	auto now_point = period.count();

	std::cout << "�̱� ������ �ɸ� �ð� (�и���): " << now_point << ".\n";
	std::cout << "��� ��: " << Massive_Number << "\n";

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

	std::cout << "��Ƽ������ �ɸ� �ð� (�и���): " << now_point << ".\n";
	std::cout << "��� ��: " << Massive_Number << "\n";

	//worker.join(); // ������ Worker�� �۵��� �߰��� �ߴܵȴ�.

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

