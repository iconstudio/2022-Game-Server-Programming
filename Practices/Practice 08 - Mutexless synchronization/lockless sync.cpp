#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>

//volatile int sum = 0;
// 원자적 변수는 파이프라인의 Out of order 문제를 해결한다.
std::atomic_int sum = 0;

volatile int victim = 0;
volatile bool flag[2] = { false, false };

volatile bool done = false;
volatile int* bound;
int error = 0;

void AtomicWorker();

void PetersonLock(int myID);
void PetersonUnlock(int myID);
void PetersonWorker(int myID);

void WatcherWorker1();
void WatcherWorker2();

int main()
{
	int test_bound[32];

	memset((void*)(test_bound), 0, sizeof(test_bound));
	long long addr = (long long)(&test_bound[31]);

	int offset = addr % 64;
	addr = addr - offset; //(addr / 64) * 64;
	addr = addr - 2;
	bound = (int*)(addr);
	*bound = 0;

	//std::thread worker1{ PetersonWorker, 0 };
	//std::thread worker2{ PetersonWorker, 1 };
	std::thread worker1{ AtomicWorker };
	std::thread worker2{ AtomicWorker };
	//std::thread worker1{ WatcherWorker1 };
	//std::thread worker2{ WatcherWorker2 };

	worker1.join();
	worker2.join();

	std::cout << "Summary: " << sum << std::endl;

	return 0;
}

void AtomicWorker()
{
	int local_sum = 0;

	for (int i = 0; i < 500000000; ++i)
	{
		sum += 1;
		//local_sum += 1;
	}
}

void PetersonWorker(int myID)
{
	int local_sum = 0;

	for (int i = 0; i < 500000000; ++i)
	{
		local_sum += 1;
	}

	PetersonLock(myID);
	sum += local_sum;
	PetersonUnlock(myID);
}

void WatcherWorker1()
{
	for (int j = 0; j <= 25000000; ++j)
	{
		*bound = -(1 + *bound);
	}

	done = true;
}

void WatcherWorker2()
{
	while (!done)
	{
		int v = *bound;
		if ((v != 0) && (v != -1))
		{
			std::cout << "Error: " << ++error << " (v: " << std::hex << v << ")\n";
		}
	}

	std::cout << "Last Error: " << error << std::endl;
}

void PetersonLock(int myID)
{
	int other = 1 - myID;
	flag[myID] = true;
	victim = myID;

	// 파이프라인 문제를 mfence 명령어로 방지한다.
	// 이후의 코드의 순서를 현재 파이프라인 뒤로 미룬다.
	//_asm mfence;

	//std::atomic_thread_fence(std::memory_order_seq_cst);

	while (flag[other] && victim == myID) {}
}

void PetersonUnlock(int myID)
{
	flag[myID] = false;
}