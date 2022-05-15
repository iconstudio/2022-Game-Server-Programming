#pragma once
#include "stdafx.h"

void ClearOverlap(LPWSAOVERLAPPED overlap);

using PID = ULONG;
constexpr USHORT PORT = 6000;
constexpr UINT CLIENTS_MAX_NUMBER = 10;
constexpr PID CLIENTS_ORDER_BEGIN = 10000;
constexpr UINT BUFFSIZE = 512;

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
	SC_APPEAR_CHARACTER,
	SC_DISAPPEAR_CHARACTER,
	SC_SIGNOUT
};

#pragma pack(push, 1)
class EXOVERLAPPED : public WSAOVERLAPPED
{
public:
	EXOVERLAPPED(OVERLAP_OPS operation);
	~EXOVERLAPPED();

	void SetSendBuffer(const WSABUF& buffer);
	void SetSendBuffer(LPWSABUF buffer);
	void SetSendBuffer(CHAR* cbuffer, DWORD size);

	const OVERLAP_OPS Operation;
	PACKET_TYPES Type;

	std::unique_ptr<WSABUF> sendBuffer;
	std::unique_ptr<CHAR> sendCBuffer;
	DWORD sendSize, sendSzWant;
};

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
	SCPacketCreateCharacter(PID pid, INT cx, INT cy);

	INT x, y;
};

/// <summary>
/// Ư�� �÷��̾��� ĳ���� �̵�
/// </summary>
struct SCPacketMoveCharacter : public Packet
{
	SCPacketMoveCharacter(PID pid, INT nx, INT ny);

	INT x, y;
};

/// <summary>
/// Ư�� �÷��̾��� ĳ���� ��Ÿ����
/// </summary>
struct SCPacketAppearCharacter : public Packet
{
	SCPacketAppearCharacter(PID tid, INT nx, INT ny);

	INT x, y;
};

/// <summary>
/// Ư�� �÷��̾��� ĳ���� �������
/// </summary>
struct SCPacketDisppearCharacter : public Packet
{
	SCPacketDisppearCharacter(PID tid);
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
