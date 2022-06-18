#pragma once

void AcceptCallback(Framework& framework, Socket& socket, const NetErrorCode& ec);
void RecvCallback(Framework& framework, PID id, Socket& socket, const NetErrorCode& ec);
void SendCallback(Framework& framework, PID id, Socket& socket, const NetErrorCode& ec);

class Framework
{
public:
	Framework();
	~Framework();

	void Awake();
	void Start();
	void Update(float delta_time);
	void Release();

	shared_atomic<ClientSlot> CreateSession(Socket& socket, size_t place);

	NetService myService;
	NetGate myAcceptor;
	NetAddress myAddress;

	atomic_flag cvClientAccept;
	atomic_flag cvClientModify;
	atomic_flag cvClientDB;
	atomic_flag cvSight;
	atomic_flag cvNPCUpdate;

	std::vector<std::jthread> thWorkers;
	std::jthread thTimer;
	std::jthread thDB;

	std::array<shared_atomic<ClientSlot>, 200000> myClients;
	concurrent_pqueue<shared_atomic<ClientSlot>> myUsers;

	std::unordered_map<std::string, shared_ptr<Sprite>> mySprites;
};
