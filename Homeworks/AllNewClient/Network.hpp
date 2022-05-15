#pragma once

class Network
{
public:
	Network(const ULONG max_clients);
	~Network();

	void Awake();
	void Start();
	void Update();

	std::optional<Packet>& OnReceive(DWORD bytes);
	void OnSend(LPWSAOVERLAPPED asynchron, DWORD bytes);

private:
	inline SOCKET CreateSocket() const;

	string serverIP;
	short serverPort;

	const ULONG clientsMax;

	SOCKET mySocket;
	SOCKADDR_IN serverAddress;
	int serverAddressSize;

	Asynchron recvOverlap;
	WSABUF recvBuffer;
	CHAR recvCBuffer[BUFFSZ];
	DWORD recvBytes;
};
