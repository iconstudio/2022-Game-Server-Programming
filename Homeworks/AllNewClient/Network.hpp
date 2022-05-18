#pragma once
#include "Asynchron.hpp"
#include "Packet.hpp"
#include "Session.hpp"

void CallbackRecv(DWORD error, DWORD bytes, LPWSAOVERLAPPED overlap, DWORD flags);
void CallbackSend(DWORD error, DWORD bytes, LPWSAOVERLAPPED overlap, DWORD flags);

constexpr USHORT PORT = 6000;
constexpr PID CLIENTS_ORDER_BEGIN = 10000;
enum class NETWORK_STATES : UCHAR;

class unique_barrier
{
public:
	unique_barrier(atomic_bool& boolean, bool init_flag);
	~unique_barrier();

	const bool myFlag;
	atomic_bool& myBoolean;
};

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

	void AddLocalSession(PID id, const char* nickname);
	/// <summary>
	/// 플레이어, NPC, 상호작용 객체 등록
	/// </summary>
	/// <param name="id"></param>
	void AddLocalEntity(PID id, int type, int x, int y);

	bool IsPlayer(PID id) const;
	bool IsNonPlayer(PID id) const;

	std::optional<Packet*> OnReceive(DWORD bytes);
	std::optional<Packet*> OnSend(LPWSAOVERLAPPED asynchron, DWORD bytes);

	NETWORK_STATES myStatus;
	struct
	{
		string myNickname;
		PID myID = -1;
	} myProfile;

private:
	int Receive(DWORD begin_bytes = 0);
	int Send(LPWSABUF datas, UINT count, LPWSAOVERLAPPED asynchron);
	int SendPacket(Packet* packet);

	inline SOCKET CreateSocket() const;

	inline auto FindPlayer(const PID id) const;
	inline void RegisterPlayer(const PID id);
	inline void RemovePlayer(const PID id);

	SOCKET mySocket;
	string serverIP;
	short serverPort;
	SOCKADDR_IN serverAddress;
	int serverAddressSize;

	atomic_bool mySemaphore;

	// 최대 접속자 수
	const ULONG clientsMax;
	// 접속한 모든 플레이어 목록
	std::unordered_map<PID, shared_ptr<Session>> myLocalClients;

	Asynchron recvOverlap;
	WSABUF recvBuffer;
	CHAR recvCBuffer[BUFFSZ];
	DWORD recvBytes;
};

enum class NETWORK_STATES : UCHAR
{
	CLOSED = 0,
};
