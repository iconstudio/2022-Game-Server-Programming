#include <iostream>
#include <thread>
#include <atomic>
#include <memory>
#include <chrono>
#include <concurrent_unordered_map.h>

using namespace std::chrono;

int sum = 0;



int main()
{
	concurrency::concurrent_unordered_map<int, int> my_map;

	std::atomic<std::shared_ptr<bool>> shared_bool1{};
	std::atomic<std::shared_ptr<bool>> shared_bool2{};

	shared_bool1.store(false);
	shared_bool2.store(shared_bool1);

	shared_bool1.store(true);
	shared_bool1.compare_exchange_strong(false, true);

	auto start = high_resolution_clock::now();


	auto duration = high_resolution_clock::now() - start;

	std::cout << "Sum: " << sum;
	std::cout << " Duration: " << duration_cast<milliseconds>(duration).count() << " milliseconds\n";

	return 0;
}
