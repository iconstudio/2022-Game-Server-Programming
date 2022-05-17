#pragma once

class ServerFramework
{
public:
	ServerFramework();
	~ServerFramework();

	void Awake();
	void Start();

private:
	const ULONG_PTR serverID;

	SOCKET Listener;
	SOCKADDR_IN Address;
	INT szAddress;
	HANDLE completionPort;
	std::vector<std::thread> threadWorkers;

	WSAOVERLAPPED acceptOverlap;
	DWORD acceptBytes;
	char acceptCBuffer[BUFSIZ];
	atomic<SOCKET> acceptNewbie;

	std::array<shared_ptr<Session>, CLIENTS_MAX_NUMBER> clientsPool;
	atomic<PID> orderClientIDs;
	atomic<UINT> numberClients;
	std::timed_mutex mutexClient;
};
