#include <iostream>
#include <chrono>
#include <thread>

using namespace std;
using namespace chrono;

#define _PRACTICE_CACHE_MISS

int main()
{
#ifdef _PRACTICE_SYSTEM_CALL
	volatile long long tmp = 0;
	auto start = high_resolution_clock::now();

	for (int j = 0; j < 10000000; ++j)
	{
		tmp += j;
		//this_thread::yield(); // 시스템 호출 (다른 스레드에 양보)
	}

	auto duration = high_resolution_clock::now() - start;
	cout << "Time " << duration_cast<milliseconds>(duration).count();
	cout << " msec\n";
	cout << "RESULT " << tmp << endl;
#endif

#ifdef _PRACTICE_CACHE_MISS
	constexpr int CACHE_LINE_SIZE = 32;

	for (int i = 0; i < 20; ++i)
	{
		const long size = 1024 << i;
		long long* a = (long long*)malloc(size); // 배열
		//memset(&a, 0, size);

		unsigned int index = 0;
		long long tmp = 0;
		const int num_data = size / 8;
		auto start = high_resolution_clock::now();

		for (int j = 0; j < 100000000; ++j)
		{
			// 배열이 너무 커서 캐시 안에 담을 수 없다.
			// Surface Go2: 128kb / 512kb / 4mb

			/*
				여러 캐시를 읽게 만들어야 미스가 날 확률이 높아진다.
				하나의 캐시만 사용하면 무조건 미스가 나지 않는다.
				모든 캐시를 골고루 건드린다. 단순히 여러개의 배열 원소를 참조하는 건 안된다.

				CPU가 배열을 읽으면, 보통 그 다음 원소도 읽는다.
				그래서 이런 예측을 못하게 읽는다.
			*/
			tmp += a[index % num_data];
			index += CACHE_LINE_SIZE * 11;
		}

		auto dur = high_resolution_clock::now() - start;
		auto ms = duration_cast<milliseconds>(dur).count();

		cout << "Size (byte): " << size << '\n';
		cout << "Result: " << tmp << '\n';
		cout << "Duration (ms): " << ms << '\n' << endl;
	}
#endif
}
