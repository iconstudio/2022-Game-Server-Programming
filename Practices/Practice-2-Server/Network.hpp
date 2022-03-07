#pragma once
#include "Client.hpp"

using BIT = char;

#pragma pack(1)
struct NetPacketMin {
	u_int type;
};
struct NetPacket {
	u_int type;
	SOCKET socket;
	u_int sz_buffer;
	BIT buffer[SZ_BUFF];
};
#pragma pack()

class NetworkServer {
public:
	NetworkServer();
	virtual ~NetworkServer();

	void Start(u_short port = PORT_DEFAULT);
	int Receive(std::shared_ptr<Client>);
	int Send(std::shared_ptr<Client>, BIT*, u_int);

	void SetActiveAccept(bool);

	bool IsConnected();

	static constexpr u_short PORT_DEFAULT = 9000;

	static void CleanSocket(SOCKET sk);
private:
	struct Cabinet {
		WSADATA wsa;
		SOCKET socket;
		SOCKADDR_IN address;
		int sz_address;
		u_short port;

		Cabinet();

		void Clear();
		void SetNetPort(u_short);
		void Init();
		void Bind();
		void Listen();
		SOCKET Accept();
		void Close();
	} cabinet;
	friend struct Cabinet;

	std::promise<bool> connection_flag;
	std::shared_future<bool> connection;
	std::atomic_bool acceptable;
	std::mutex accept_critical_section;
	std::unique_lock<std::mutex> accept_lock;
	std::vector<std::shared_ptr<Client>> clients;
	std::unique_ptr<std::thread> proc_accept;
	//std::map<SOCKET, std::shared_ptr<Client>> clients;

	void Initiate(u_short port);
	void Connect();
	void Open();
	void Accept(SOCKET);
	void Discard(SOCKET);

	void ListenWorker();
	void AcceptWorker(std::promise<SOCKET>*);
	void ConnectWorker();
	void CommunicationWorker(std::shared_ptr<Client>);

	void SetConnectionState(bool);

	class weak_error : public std::exception {
	public:
		using _Mybase = exception;
		explicit weak_error(const std::string& _Message) : _Mybase(_Message.c_str()) {}
		explicit weak_error(const char* _Message) : _Mybase(_Message) {}
	};
};

