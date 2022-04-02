#pragma once
#pragma comment(lib, "MSWSock.lib")
#pragma comment(lib, "Ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WS2tcpip.h>
#include <MSWSock.h>

#include <iostream>
#include <vector>
#include <concurrent_vector.h>
#include <concurrent_unordered_map.h>
#include <algorithm>
#include <tuple>
using namespace std;

class IOCPFramework;

enum class OVERLAP_OPS
{

};

class EXOVERLAPPED : public WSAOVERLAPPED
{
	SOCKET Socket;
};

#pragma pack(push, 1)
enum PACKET_TYPES : USHORT
{
	NONE = 0,
	CS_SIGNIN,
	CS_SIGNOUT,
	CS_KEY,
	SC_SIGNUP,
	SC_CREATE_CHARACTER,
	SC_MOVE_CHARACTER,
	SC_SIGNOUT
};

using PID = UINT;
struct Packet
{
	Packet(PACKET_TYPES type, USHORT size, PID pid);
	Packet(PACKET_TYPES type, PID pid = 0);

	USHORT Size;
	PACKET_TYPES Type;
	PID playerID;
};

/// <summary>
/// 서버에 로그인 의사 알림
/// </summary>
struct CSPacketSignIn : public Packet
{
	CSPacketSignIn(const CHAR* nickname);

	CHAR Nickname[30];
};

/// <summary>
/// 서버에서 나가는 의사 알림
/// </summary>
struct CSPacketSignOut : public Packet
{
	CSPacketSignOut(PID pid);
};

/// <summary>
/// 서버에 입력한 키 알림
/// </summary>
struct CSPacketKeyInput : public Packet
{
	CSPacketKeyInput(PID pid, WPARAM key);

	WPARAM Key;
};

/// <summary>
/// 새로 접속한 플레이어에게 ID 부여, 현재 동접자 수, 최대 동접자 수 알리기
/// </summary>
struct SCPacketSignUp : public Packet
{
	SCPacketSignUp(PID nid, UINT users = 0, UINT usersmax = 0);

	UINT usersCurrent, usersMax;
};

/// <summary>
/// 특정 플레이어의 캐릭터 생성
/// </summary>
struct SCPacketCreateCharacter : public Packet
{
	SCPacketCreateCharacter(PID pid, UCHAR cx, UCHAR cy);

	UCHAR x, y;
};

/// <summary>
/// 특정 플레이어의 캐릭터 이동
/// </summary>
struct SCPacketMoveCharacter : public Packet
{
	SCPacketMoveCharacter(PID pid, UCHAR nx, UCHAR ny);

	UCHAR x, y;
};

/// <summary>
/// 특정 플레이어의 캐릭터 삭제 (나간 플레이어 이외에 다른 플레이어에 전송)
/// </summary>
struct SCPacketSignOut : public Packet
{
	SCPacketSignOut(PID pid);
};

/// <summary>
/// 플레이어 캐릭터
/// </summary>
class PlayerCharacter
{
public:
	bool TryMoveLT();
	bool TryMoveRT();
	bool TryMoveUP();
	bool TryMoveDW();

	UCHAR x, y;
};
#pragma pack(pop)

void ClearOverlap(LPWSAOVERLAPPED overlap);
void ErrorDisplay(const char* title);

constexpr USHORT PORT = 6000;
constexpr UINT CLIENTS_MAX_NUMBER = 10;
constexpr PID CLIENTS_ORDER_BEGIN = 10000;
constexpr UINT BUFFSIZE = 512;
constexpr SIZE_T WND_SZ_W = 800, WND_SZ_H = 600; // 창 크기
constexpr SIZE_T CELL_SIZE = 64;
constexpr int CELL_W = CELL_SIZE;
constexpr int CELL_H = CELL_SIZE;
constexpr int CELLS_CNT_H = 8;
constexpr int CELLS_CNT_V = 8;
constexpr int CELLS_LENGTH = CELLS_CNT_H * CELLS_CNT_V;
constexpr int BOARD_W = CELL_W * CELLS_CNT_H;
constexpr int BOARD_H = CELL_H * CELLS_CNT_V;
constexpr int BOARD_X = (WND_SZ_W - BOARD_W) / 2;
constexpr int BOARD_Y = (WND_SZ_H - BOARD_H - 20) / 2;
