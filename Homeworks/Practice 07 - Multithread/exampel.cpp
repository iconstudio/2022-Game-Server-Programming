#include "multithread.hpp"

using namespace std;
using namespace std::chrono;

volatile int sum;
constexpr auto MAX_THREADS = 16;

std::mutex SafeLock{};

void thread_func(int num_threads)
{
	volatile int local_sum = 0; // ���� ���� �ȿ��� ��� �۾��� �Ϸ��Ѵ�.

	// SafeLock.lock();
	// ���� �ִ� �Ӱ� ������ ������ �Ű��.
	for (auto i = 0; i < 50000000 / num_threads; ++i)
	{
		local_sum += 2;
	}

	SafeLock.lock();
	sum += local_sum; // �Ӱ� ���� ���� �ڵ� ���� �������ν� ���ϸ� ���� �� �ִ�.
	SafeLock.unlock();
}

int main()
{
	vector<thread> threads;

	// ������ȭ ������ �Ѵ� ���غ��� �Ѵ�!
	//for (auto i = 1; i <= MAX_THREADS; i *= 2)
	for (auto i = MAX_THREADS; 0 < i; i /= 2)
	{
		sum = 0;
		threads.clear();

		auto start = high_resolution_clock::now();
		for (auto j = 0; j < i; ++j)
		{
			threads.push_back(thread{ thread_func, i });
		}

		for (auto& tmp : threads)
		{
			tmp.join();
		}

		auto duration = high_resolution_clock::now() - start;

		cout << i << " Threads, Sum: " << sum;
		cout << " Duration: " << duration_cast<milliseconds>(duration).count() << " milliseconds\n";
	}
}
