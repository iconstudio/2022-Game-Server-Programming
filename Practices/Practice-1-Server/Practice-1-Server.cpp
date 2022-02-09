#include "pch.h"
#include "ServerFramework.h"

constexpr int PORT = 40000;


void _Server_start(promise<bool>*&);
void _Server_update();

int main() {
	std::cout << "Server initiated.\n";

	auto initiator = new promise<bool>();
	auto awaiter = initiator->get_future();

	auto init_worker = thread(_Server_start, &initiator);
	
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

void _Server_start(promise<bool>*& fallback) {
	WSADATA wsadata;

	try {
		auto result = WSAStartup(MAKEWORD(2, 2), &wsadata);
		if (0 != result) {
			fallback->set_exception(make_exception_ptr("Startup error!"));
		} else {
			fallback->set_value(true);
		}

		auto listener = socket(AF_INET, SOCK_STREAM, 0);
		if (INVALID_SOCKET == listener)
			fallback->set_exception(make_exception_ptr("Socket creation error!"));

		SOCKADDR_IN server_address;
		ZeroMemory(&server_address, sizeof(server_address));
		server_address.sin_family = AF_INET;
		server_address.sin_addr.s_addr = htonl(INADDR_ANY);
		server_address.sin_port = htons(PORT);

		result = bind(listener, (SOCKADDR*)(&server_address), sizeof(server_address));
		if (SOCKET_ERROR == result)
			fallback->set_exception(make_exception_ptr("Addressing error!"));

		result = listen(listener, SOMAXCONN);
		if (SOCKET_ERROR == result)
			fallback->set_exception(make_exception_ptr("Listening error!"));
	} catch (...) {
		fallback->set_exception(std::current_exception());
	}
}
