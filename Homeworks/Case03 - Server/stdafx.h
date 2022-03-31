#pragma once
#pragma comment(lib, "MSWSock.lib")
#pragma comment(lib, "Ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <MSWSock.h>
#include <WS2tcpip.h>

#include <iostream>
#include <vector>
#include <concurrent_vector.h>
#include <concurrent_unordered_map.h>
#include <algorithm>
#include <atomic>
using namespace std;

class IOCPServerFramework;

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
	Packet(PACKET_TYPES type, USHORT size, UINT pid);
	Packet(PACKET_TYPES type, UINT pid = 0);

	USHORT Size;
	PACKET_TYPES Type;
	UINT playerID;
};

/// <summary>
/// ������ �α��� �ǻ� �˸�
/// </summary>
struct CSPacketSignIn : public Packet
{
	CSPacketSignIn(const CHAR* nickname);

	CHAR Nickname[30];
};

/// <summary>
/// �������� ������ �ǻ� �˸�
/// </summary>
struct CSPacketSignOut : public Packet
{
	CSPacketSignOut(UINT pid);
};

/// <summary>
/// ������ �Է��� Ű �˸�
/// </summary>
struct CSPacketKeyInput : public Packet
{
	CSPacketKeyInput(UINT pid, WPARAM key);

	WPARAM Key;
};

/// <summary>
/// ���� ������ �÷��̾�� ID �ο�
/// </summary>
struct SCPacketSignUp : public Packet
{
	SCPacketSignUp(UINT nid);
};

/// <summary>
/// Ư�� �÷��̾��� ĳ���� ����
/// </summary>
struct SCPacketCreateCharacter : public Packet
{
	SCPacketCreateCharacter(UINT pid, UCHAR cx, UCHAR cy);

	UCHAR x, y;
};

/// <summary>
/// Ư�� �÷��̾��� ĳ���� �̵�
/// </summary>
struct SCPacketMoveCharacter : public Packet
{
	SCPacketMoveCharacter(UINT pid, UCHAR nx, UCHAR ny);

	UCHAR x, y;
};

/// <summary>
/// Ư�� �÷��̾��� ĳ���� ���� (���� �÷��̾� �̿ܿ� �ٸ� �÷��̾ ����)
/// </summary>
struct SCPacketSignOut : public Packet
{
	SCPacketSignOut(UINT pid);
};

/// <summary>
/// �÷��̾� ĳ����
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

void ErrorDisplay(const char* title);

constexpr UINT CLIENTS_MAX_NUMBER = 10;
constexpr UINT BUFFSIZE = 512;
constexpr SIZE_T WND_SZ_W = 800, WND_SZ_H = 600; // â ũ��
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
