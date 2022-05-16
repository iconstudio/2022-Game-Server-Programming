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

	int SendSignInMsg();
	int SendSignOutMsg();
	int SendKeyMsg(WPARAM key);

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
	int SendPacket(Packet* packet);

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

	// 최대 접속자 수
	const ULONG clientsMax;
	// 접속한 모든 플레이어 목록
	std::unordered_map<PID, shared_ptr<Session>> myLocalClients;
	// 시야에 보이는 객체 목록 (시야)
	std::vector<shared_ptr<GameEntity>> myLocalInstances;

	Asynchron recvOverlap;
	WSABUF recvBuffer;
	CHAR recvCBuffer[BUFFSZ];
	DWORD recvBytes;
};

enum class NETWORK_STATES : UCHAR
{
	CLOSED = 0,
};
