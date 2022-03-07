#include "pch.hpp"
#include "Network.hpp"
#include "Client.hpp"

NetworkServer::NetworkServer()
	: connection_flag(), connection(connection_flag.get_future())
	, acceptable(false), proc_accept(nullptr)
	, accept_critical_section(), accept_lock(accept_critical_section) {
	accept_lock.unlock();
	cabinet.Clear();
}

NetworkServer::~NetworkServer() {
	cabinet.Close();
	accept_lock.release();
	WSACleanup();
}

void NetworkServer::Start(u_short port) {
	Initiate(port);
	Connect();
	Open();
	SetActiveAccept(false);

	while (true) {

	}
}

int NetworkServer::Receive(std::shared_ptr<Client> client) {
	var sk = client->socket;
	var& bf = client->buffer;
	var& sz = client->sz_recv;
	var& wb = client->wbuffer;
	var& ov = client->GetOverlap();
	DWORD flags = 0;

	var result = WSARecv(sk, &wb, 1, &sz, &flags, &ov, NULL); 
	if (SOCKET_ERROR == result) {
		var err = WSAGetLastError();
		if (WSA_IO_PENDING != err) {
			Discard(sk);
			throw weak_error("Cannot Receive");
		}
	}

	return result;
}

int NetworkServer::Send(std::shared_ptr<Client> client, BIT* data, u_int sz) {
	return 0;
}

void NetworkServer::SetActiveAccept(bool value) {
	if (accept_lock.try_lock()) {
		if (acceptable != value) {
			acceptable = value;

			if (acceptable) {

			}
		}

		accept_lock.unlock();
	}
}

bool NetworkServer::IsConnected() {
	return connection.get();
}

void NetworkServer::Initiate(u_short port) {
	try {
		cabinet.Init();
	} catch (std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}

	std::cout << "The server is starting...\n";
}

void NetworkServer::Connect() {
	auto worker = std::thread(&NetworkServer::ConnectWorker, this);
	auto period = std::chrono::seconds(3);
	auto status = connection.wait_for(period);

	if (std::future_status::ready == status) {
		try {
			if (IsConnected()) {
				std::cout << "The server is initiated.\n";
			} else {
				std::cerr << "Cannot open the server.\n";
			}
		} catch (std::exception& e) {
			std::cerr << "Fatal Error: " << e.what() << std::endl;
		}
	}

	worker.join();
}

void NetworkServer::Open() {
	std::cout << "Server is opened.\n";

	var th = std::thread(&NetworkServer::ListenWorker, this);
	th.join();
}

void NetworkServer::Accept(SOCKET sk) {
	if (accept_lock.try_lock()) {
		auto client = std::make_shared<Client>(sk);

		auto wevent = WSACreateEvent();
		if (WSA_INVALID_EVENT == wevent) {
			throw weak_error("Cannot create an event!");
		}
		client->SetEvent(std::move(wevent));

		var sk = client->socket;

		clients.push_back(std::move(client));
		std::cout << "Client is connected: " << sk << "\n";
		accept_lock.unlock();
	} else {
	}
}

void NetworkServer::Discard(SOCKET sk) {
	var it = std::find_if(clients.begin(), clients.end(), [sk](auto& client) {
		return (client->socket == sk);
	});

	if (it != clients.end()) {
		CleanSocket((*it)->socket);
		clients.erase(it);
	}
}

void NetworkServer::ListenWorker() {
	std::promise<SOCKET> newbie;
	auto waiter = newbie.get_future();

	var th = new std::thread(&NetworkServer::AcceptWorker, this, &newbie);
	proc_accept.reset(th);

	try {
		while (true) {
			std::cout << "Waiting a client...\n";
			auto status = waiter.wait_for(std::chrono::seconds(1));

			if (acceptable) {
				if (std::future_status::timeout == status) {
					continue;
				} else if (std::future_status::ready == status) {
					SOCKET cl_socket;

					try {
						cl_socket = waiter.get();
						Accept(cl_socket);
					} catch (weak_error& e) {
						if (INVALID_SOCKET != cl_socket) {
							CleanSocket(cl_socket);
						}

						std::cout << "Error: " << e.what() << std::endl;
						continue;
					}
				}
			}
		}
	} catch (std::exception& e) {
		std::cerr << "Fatal Error: " << e.what() << std::endl;
	}

	proc_accept->join();
}

void NetworkServer::AcceptWorker(std::promise<SOCKET>* newbie) {
	while (true) {
		if (acceptable) {
			SOCKET cl_socket;
			try {
				cl_socket = cabinet.Accept();
			} catch (...) {
				newbie->set_exception(std::current_exception());

				return;
			}

			newbie->set_value(cl_socket);
		}
	}
}

void NetworkServer::ConnectWorker() {
	try {
		cabinet.Bind();
		cabinet.Listen();
	} catch (...) {
		connection_flag.set_exception(std::current_exception());
		SetConnectionState(false);
		return;
	}
	SetConnectionState(true);
}

void NetworkServer::CommunicationWorker(std::shared_ptr<Client> client) {
	var sk = client->socket;
	var& bf = client->buffer;
	var& sz = client->sz_recv;
	var& wb = client->wbuffer;
	var& ov = client->GetOverlap();
	DWORD transfer, flags = 0;

	while (true) {
		var result = WSAGetOverlappedResult(sk, &ov, &transfer, FALSE, &flags);
		if (FALSE == result || 0 == transfer) {
			// 접속 종료
			continue;
		}


	}
}

void NetworkServer::SetConnectionState(bool value) {
	connection_flag.set_value(value);
}

NetworkServer::Cabinet::Cabinet()
	: wsa(), socket(NULL)
	, address(), sz_address(sizeof(address))
	, port(PORT_DEFAULT) {}

void NetworkServer::Cabinet::Clear() {
	ZeroMemory(&address, sz_address);
}

void NetworkServer::Cabinet::SetNetPort(u_short port) {
	this->port = port;
}

void NetworkServer::Cabinet::Init() {
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsa)) {
		throw std::runtime_error("Startup Error");
	}

	socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == socket) {
		throw std::runtime_error("Socket Creation Error");
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(port);
}

void NetworkServer::Cabinet::Bind() {
	auto ptr_address = reinterpret_cast<SOCKADDR*>(&address);

	if (SOCKET_ERROR == bind(socket, ptr_address, sz_address)) {
		throw std::runtime_error("Socket Binding Error");
	}
}

void NetworkServer::Cabinet::Listen() {
	if (SOCKET_ERROR == listen(socket, SOMAXCONN)) {
		throw std::runtime_error("Socket Listening Error");
	}
}

SOCKET NetworkServer::Cabinet::Accept() {
	auto ptr_address = reinterpret_cast<SOCKADDR*>(&address);

	var cl_socket = WSAAccept(socket, ptr_address, &sz_address, NULL, NULL);
	if (INVALID_SOCKET == cl_socket) {
		CleanSocket(cl_socket);
		throw weak_error("Client Connection Error");
	}

	return cl_socket;
}

void NetworkServer::Cabinet::Close() {
	closesocket(socket);
}

void NetworkServer::CleanSocket(SOCKET sk) {
	WSASendDisconnect(sk, NULL);
	closesocket(sk);
}
