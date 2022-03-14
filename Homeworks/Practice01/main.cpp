#include <vector>
#include <chrono>
#include <algorithm>
#include <string>
#include <iostream>
#include <map>
#include <algorithm>
#include <atomic>

using namespace std;
using namespace chrono;

constexpr int SIZE = 100000000;

#define abs(x) (0 < x) ? (x) : -(x)

constexpr long ABS(const long value)
{
	long rhs = value >> 31;
	return (value ^ rhs) - rhs;
}

int main()
{
	long sum = 0;
	long *rnd_arr = new long[SIZE];

	for (int i = 0; i < SIZE; ++i)
	{
		rnd_arr[i] = rand() - 16384;
	}

	auto start = high_resolution_clock::now();

	for (int j = 0; j < SIZE; ++j)
	{
		sum += abs(rnd_arr[j]);
	}

	auto period = high_resolution_clock::now() - start;
	auto duration = duration_cast<milliseconds>(period).count();

	cout << "Time (macro): " << duration << '\n';
	cout << "Summary: " << sum << endl;
	sum = 0;

	start = high_resolution_clock::now();

	for (int j = 0; j < SIZE; ++j)
	{
		sum += ABS(rnd_arr[j]);
	}

	period = high_resolution_clock::now() - start;
	duration = duration_cast<milliseconds>(period).count();

	cout << "Time (func): " << duration << '\n';
	cout << "Summary: " << sum << endl;


	return 0;
}

