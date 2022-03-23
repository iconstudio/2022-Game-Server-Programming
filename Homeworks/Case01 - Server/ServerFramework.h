#pragma once

class ServerFramework
{
public:
	~ServerFramework();
	void Init();
	void Start();
	friend void CallbackStartPositions(DWORD, DWORD
		, LPWSAOVERLAPPED, DWORD);
	friend void CallbackInputs(DWORD, DWORD
		, LPWSAOVERLAPPED, DWORD);

	SOCKET Socket;
	SOCKADDR_IN Address;
	INT sz_Address;
	WSAOVERLAPPED Overlap_recv;

private:
	bool AcceptSession();
	Player* CreatePlayerCharacter(Session* session);
	void RemoveSession(const INT id);

	INT Clients_index = 0;
	unordered_map<INT, Session*> Clients;
	unordered_map<LPWSAOVERLAPPED, Session*> Overlients;
};

struct Position
{
	int x, y;
};

class Player : public Position
{
public:
	bool TryMoveLT();
	bool TryMoveRT();
	bool TryMoveUP();
	bool TryMoveDW();
};
