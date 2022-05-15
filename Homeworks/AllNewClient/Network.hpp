#pragma once

class Network
{
public:
	Network(const ULONG max_clients);
	~Network();

	void Awake();
	void Start();
	void Update();

	void OnReceive(DWORD bytes);
	void OnSend(LPWSAOVERLAPPED asynchron, DWORD bytes);

private:
	inline SOCKET CreateSocket() const;

	string serverIP;
	short serverPort;

	const ULONG clientsMax;

	SOCKET mySocket;
	SOCKADDR_IN serverAddress;
	int serverAddressSize;

	static const size_t RECV_MAX = 512;
	Asynchron recvOverlap;
	WSABUF recvBuffer;
	CHAR recvCBuffer[RECV_MAX];
	DWORD recvBytes;
};
