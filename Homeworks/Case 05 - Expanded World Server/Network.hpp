#pragma once
#include "stdafx.hpp"

using PID = ULONG;
constexpr USHORT PORT = 6000;
constexpr UINT BUFFSIZE = 512;

constexpr UINT CLIENTS_MAX_NUMBER = 5000;
constexpr PID CLIENTS_ORDER_BEGIN = 10000;

constexpr float CELL_H = 16.0f;
constexpr float CELL_V = 16.0f;

constexpr size_t SIGHT_CELLS_CNT_H = 11;
constexpr size_t SIGHT_CELLS_CNT_V = 11;
constexpr float SIGHT_PX_SZ_H = CELL_H * SIGHT_CELLS_CNT_H;
constexpr float SIGHT_PX_SZ_V = CELL_V * SIGHT_CELLS_CNT_V;

constexpr size_t WORLD_CELLS_CNT_H = 400;
constexpr size_t WORLD_CELLS_CNT_V = 400;
constexpr float WORLD_PX_SZ_H = CELL_H * WORLD_CELLS_CNT_H;
constexpr float WORLD_PX_SZ_V = CELL_V * WORLD_CELLS_CNT_V;

enum class PACKET_TYPES : UCHAR
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

#pragma pack(push, 1)
class Packet
{
public:
	Packet(PACKET_TYPES type, USHORT size, PID pid);
	Packet(PACKET_TYPES type, PID pid = 0);

	USHORT Size;
	PACKET_TYPES Type;
	PID playerID;
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
	CSPacketSignOut(PID pid);
};

/// <summary>
/// ������ �Է��� Ű �˸�
/// </summary>
struct CSPacketKeyInput : public Packet
{
	CSPacketKeyInput(PID pid, WPARAM key);

	WPARAM Key;
};

/// <summary>
/// ���� ������ �÷��̾�� ID �ο�, ���� ������ ��, �ִ� ������ �� �˸���
/// </summary>
struct SCPacketSignUp : public Packet
{
	SCPacketSignUp(PID nid, UINT users, UINT usersmax);

	UINT usersCurrent, usersMax;
};

/// <summary>
/// Ư�� �÷��̾��� ĳ���� ����
/// </summary>
struct SCPacketCreateCharacter : public Packet
{
	SCPacketCreateCharacter(PID pid, CHAR cx, CHAR cy);

	CHAR x, y;
};

/// <summary>
/// Ư�� �÷��̾��� ĳ���� �̵�
/// </summary>
struct SCPacketMoveCharacter : public Packet
{
	SCPacketMoveCharacter(PID pid, CHAR nx, CHAR ny);

	CHAR x, y;
};

/// <summary>
/// Ư�� �÷��̾��� ĳ���� ���� (���� �÷��̾� �̿ܿ� �ٸ� �÷��̾ ����)
/// </summary>
struct SCPacketSignOut : public Packet
{
	SCPacketSignOut(PID pid, UINT users);
	
	UINT usersCurrent;
};
#pragma pack(pop)
