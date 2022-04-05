#pragma once
#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")

#pragma warning(disable: 4244)
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <WS2tcpip.h>
#include <MSWSOCK.h>

#include <iostream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <concurrent_unordered_map.h>
#include <array>

using namespace std;

constexpr int PORT_NUM = 4000;
constexpr int BUF_SIZE = 256;
constexpr int MAX_USER = 10;
constexpr int NAME_SZ = 10;
constexpr int WORLD_SZ = 8;

enum COMPLETION_TYPES : INT
{
	ACCEPT, RECV, SEND
};

enum OPERATIONS : UCHAR
{
	NONE = 0,
	CS_LOGIN,
	CS_MOVE,

	SC_ACCEPT,
	SC_ADD_PLAYER,
	SC_RMV_PLAYER,
	SC_MOV_PLAYER,
};

#pragma pack(push, 1)
struct PACKET
{
	PACKET(USHORT size, OPERATIONS type, UINT id)
		: Size(size), Type(type), ID(id)
	{}

	USHORT Size = 0;
	OPERATIONS Type = NONE;
	UINT ID = -1;
};

struct CSPacketLogin : public PACKET
{
	CSPacketLogin(const CHAR* name, UINT id)
		: PACKET(sizeof(CSPacketLogin), CS_LOGIN, id)
		, Name()
	{
		strcpy_s(Name, name);
	}

	CHAR Name[30];
};

struct CSPacketMove : public PACKET
{
	CSPacketMove(UCHAR ax, UCHAR ay, UINT id)
		: PACKET(sizeof(CSPacketLogin), CS_MOVE, id)
		, aX(ax), aY(ay)
	{}

	UCHAR aX, aY;
};

struct SCPacketAccept : public PACKET
{
	SCPacketAccept(UINT id)
		: PACKET(sizeof(SCPacketAccept), SC_ADD_PLAYER, id)
	{}
};

struct SCPacketAddPlayer : public PACKET
{
	UINT x, y;
};

struct SCPacketRemovePlayer : public PACKET
{
	SCPacketRemovePlayer(UINT id)
		: PACKET(sizeof(SCPacketRemovePlayer), SC_RMV_PLAYER, id)
	{}
};

struct SCPacketMovePlayer : public PACKET
{
	SCPacketMovePlayer(UINT id, UINT nx, UINT ny)
		: PACKET(sizeof(SCPacketRemovePlayer), SC_MOV_PLAYER, id)
		, x(nx), y(ny)
	{}

	UINT x, y;
};
#pragma pack(pop)

struct OVER_EXP : public WSAOVERLAPPED
{
public:
	OVER_EXP() : Completion_type(RECV)
	{
		recvBuffer.len = BUF_SIZE;
		recvBuffer.buf = recvCBuffer;

		ZeroMemory(this, sizeof(WSAOVERLAPPED));
	}

	OVER_EXP(char* data, int size) : Completion_type(SEND)
	{
		sendBuffer.buf = data;
		sendBuffer.len = size;

		ZeroMemory(this, sizeof(WSAOVERLAPPED));
		CopyMemory(sendCBuffer, data, size);
	}

	COMPLETION_TYPES Completion_type;

	WSABUF recvBuffer;
	char recvCBuffer[BUF_SIZE];

	WSABUF sendBuffer;
	char sendCBuffer[BUF_SIZE];
};

class SESSION
{
public:
	SESSION()
		: _id(-1), x(0), y(0), Name()
		, _socket(NULL), _c_mess(), used(false)
		, recvOverlap(), recv_remain(0)
	{

	}

	SESSION(int id, SOCKET s) : _id(id), _socket(s)
	{
		_c_wsabuf[0].buf = _c_mess;
		_c_wsabuf[0].len = sizeof(_c_mess);
	}
	~SESSION() {}

	void do_recv()
	{
		DWORD recv_flag = 0;
		memset(&recvOverlap, 0, sizeof(recvOverlap));

		recvOverlap.recvBuffer.buf = recvOverlap.sendCBuffer + recv_remain;
		recvOverlap.recvBuffer.len = BUF_SIZE - recv_remain;


		WSARecv(_socket, _c_wsabuf, 1, 0, &recv_flag, &recvOverlap, NULL);
	}

	void do_send(void* packet)
	{
		auto sdata = new OVER_EXP(reinterpret_cast<char*>(packet), BUF_SIZE);

		WSASend(_socket, &sdata->sendBuffer, 1, 0, 0, sdata, NULL);
	}

	void SendCSPacket()
	{
		auto packet = new CSPacketLogin("Nickname", _id);
		auto data = reinterpret_cast<char*>(packet);

		do_send(data);
	}

	void SendSCMovePackets() // 모든 클라이언트가 받는다. (broadcasting)
	{
		auto packet = new CSPacketLogin("Nickname", _id);
		auto data = reinterpret_cast<char*>(packet);

		do_send(data);
	}

	SOCKET _socket;

	int _id;
	int x, y;
	CHAR Name[NAME_SZ];
	bool used;

	OVER_EXP recvOverlap;
	WSABUF _c_wsabuf[1];
	CHAR _c_mess[BUF_SIZE];
	DWORD recv_remain;
};
