#pragma once
#include "Asynchron.hpp"
#include "Packet.hpp"

void CallbackRecv(DWORD error, DWORD bytes, LPWSAOVERLAPPED overlap, DWORD flags);
void CallbackSend(DWORD error, DWORD bytes, LPWSAOVERLAPPED overlap, DWORD flags);

constexpr USHORT PORT = 6000;
constexpr PID CLIENTS_ORDER_BEGIN = 10000;
enum class NETWORK_STATES : UCHAR;

class Network
{
public:
	Network(const ULONG max_clients);
	~Network();

	void Awake();
	void Start(const char* ip);
	void Update();

	/// <summary>
	/// 플레이어, NPC, 상호작용 객체 등록
	/// </summary>
	/// <param name="id"></param>
	void AddLocalEntity(PID id, const char* name);

	bool IsPlayer(PID id) const;
	bool IsNonPlayer(PID id) const;

	std::optional<Packet> OnReceive(DWORD bytes);
	std::optional<Packet> OnSend(LPWSAOVERLAPPED asynchron, DWORD bytes);

private:
	int Receive(DWORD begin_bytes = 0);
	int Send(LPWSABUF datas, UINT count, LPWSAOVERLAPPED asynchron);

	inline SOCKET CreateSocket() const;

	NETWORK_STATES myStatus;
	struct
	{
		char myNickname[100];
		PID myID = -1;
	} myProfile;

	SOCKET mySocket;
	string serverIP;
	short serverPort;
	SOCKADDR_IN serverAddress;
	int serverAddressSize;

	const ULONG clientsMax;
	std::array<shared_ptr<Session>, CLIENTS_MAX_NUMBER> myClients;

	Asynchron recvOverlap;
	WSABUF recvBuffer;
	CHAR recvCBuffer[BUFFSZ];
	DWORD recvBytes;
};

enum class NETWORK_STATES : UCHAR
{
	CLOSED = 0,
};
