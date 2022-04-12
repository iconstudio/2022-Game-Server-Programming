#include "multithread.hpp"

using namespace std;
using namespace std::chrono;

volatile int sum;
constexpr auto MAX_THREADS = 16;

std::mutex SafeLock{};

void thread_func(int num_threads)
{
	volatile int local_sum = 0; // 스택 범위 안에서 모든 작업을 완료한다.

	// SafeLock.lock();
	// 여기 있던 임계 영역을 밑으로 옮겼다.
	for (auto i = 0; i < 50000000 / num_threads; ++i)
	{
		local_sum += 2;
	}

	SafeLock.lock();
	sum += local_sum; // 임계 영역 안의 코드 양을 줄임으로써 부하를 줄일 수 있다.
	SafeLock.unlock();
}

int main()
{
	vector<thread> threads;

	// 정진행화 역진행 둘다 비교해봐야 한다!
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
