#include "pch.h"
#include "ServerFramework.h"

constexpr int PORT = 40000;


void _Server_start(promise<bool>*&);
void _Server_update();

int main() {
	std::cout << "Server initiated.\n";

	auto initiator = new promise<bool>();
	auto awaiter = initiator->get_future();

	auto init_worker = thread(_Server_start, initiator);
	
	auto period = std::chrono::seconds(3);
	auto result = awaiter.wait_for(period);

	if (std::future_status::ready == result) {
		try {
			auto status = awaiter.get();

			if (status) {

			} else {

			}
		} catch (exception& e) {
			cout << "Error: " << e.what() << std::endl;
		}
	}
	
	init_worker.join();
}

void _Server_start(promise<bool>*& result) {
	WSADATA wsadata;

	try {
		auto status = WSAStartup(MAKEWORD(2, 2), &wsadata);
		if (0 != status) {
			result->set_exception(make_exception_ptr("Startup error!"));
		} else {
			result->set_value(true);
		}
	} catch (...) {
		result->set_exception(std::current_exception());
	}
}
