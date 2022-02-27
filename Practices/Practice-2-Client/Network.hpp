#pragma once
#include "Client.hpp"

constexpr u_int SZ_BUFFER = 512;
using BIT = char;

#pragma pack(1)
struct NetPacketMin {
	u_int type;
};
struct NetPacket {
	u_int type;
	SOCKET socket;
	u_int sz_buffer;
	BIT buffer[SZ_BUFFER];
};
#pragma pack()

class NetworkServer {
public:
	NetworkServer();
	virtual ~NetworkServer();

	void Start(u_short port = PORT_DEFAULT);

	void SetActiveAccept(bool);

	bool IsConnected();

	static constexpr u_short PORT_DEFAULT = 9000;
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

	std::vector<std::shared_ptr<Client>> clients;
	std::unique_ptr<std::thread> proc_accept;
	std::promise<bool> connection_flag;
	std::shared_future<bool> connection;
	std::atomic_bool acceptable;
	std::condition_variable accept_active;
	WSAOVERLAPPED overlap;

	void Initiate(u_short port);
	void Connect();
	void Open();
	void Accept(SOCKET);

	void ListenWorker();
	void AcceptWorker(std::promise<SOCKET>*);
	void ConnectWorker();

	void SetConnectionState(bool);

	class weak_error : public std::exception {
	public:
		using _Mybase = exception;
		explicit weak_error(const std::string& _Message) : _Mybase(_Message.c_str()) {}
		explicit weak_error(const char* _Message) : _Mybase(_Message) {}
	};
};

