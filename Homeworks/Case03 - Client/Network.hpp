#pragma once
#include "stdafx.h"

enum class OVERLAP_OPS : UCHAR
{
	NONE = 0,
	RECV,
	SEND,
};

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
	SCPacketSignUp(PID nid, UINT users = 0, UINT usersmax = 0);

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
	SCPacketSignOut(PID pid);
};
#pragma pack(pop)

constexpr USHORT PORT = 6000;
constexpr UINT CLIENTS_MAX_NUMBER = 10;
constexpr PID CLIENTS_ORDER_BEGIN = 10000;
constexpr UINT BUFFSIZE = 512;
