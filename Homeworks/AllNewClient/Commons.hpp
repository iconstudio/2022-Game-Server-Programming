#pragma once
#include "Packet.hpp"

#pragma pack(push, 1)
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
struct CSPacketMove : public Packet
{
	CSPacketMove(PID pid, UCHAR key);

	const UCHAR Key;
};

/// <summary>
/// ��ȭ �޽���, ����� -1�̸� ��ο��� ������.
/// </summary>
struct CSPacketChatMessage : public Packet
{
	CSPacketChatMessage(PID fid, PID tid, const WCHAR* msg, size_t length);
	CSPacketChatMessage(PID fid, PID tid, const WCHAR msg[]);

	PID targetID;
	WCHAR Caption[100];
};

/// <summary>
/// ���� ������ �÷��̾�� ID �ο�, ���� ������ ��, �ִ� ������ �� �˸���
/// (ù ���� �ÿ��� ����)
/// </summary>
struct SCPacketSignUp : public Packet
{
	SCPacketSignUp(PID nid, UINT users = 0, UINT usersmax = 0);

	UINT usersCurrent, usersMax;
};

/// <summary>
/// Ư�� �÷��̾��� ���� ���� ����
/// </summary>
struct SCPacketCreatePlayer : public Packet
{
	/// <param name="pid">�ٸ� �÷��̾��� ���� �ĺ���</param>
	/// <param name="nickname">����</param>
	SCPacketCreatePlayer(PID pid, const CHAR* nickname);

	CHAR Nickname[30];
};

/// <summary>
/// Ư�� ��ü�� �þ� ����
/// </summary>
struct SCPacketAppearCharacter : public Packet
{
	SCPacketAppearCharacter(PID cid, ENTITY_TYPES type, float cx, float cy);

	ENTITY_TYPES myType;
	float x, y;
};

/// <summary>
/// Ư�� ��ü�� ĳ���� �̵�
/// </summary>
struct SCPacketMoveCharacter : public Packet
{
	SCPacketMoveCharacter(PID cid, float nx, float ny);

	float x, y;
};

/// <summary>
/// Ư�� ��ü�� �þ� Ż��
/// </summary>
struct SCPacketDisppearCharacter : public Packet
{
	SCPacketDisppearCharacter(PID cid);
};

/// <summary>
/// ��ȭ �޽���, ����� -1�̸� ��ο��� ������.
/// </summary>
struct SCPacketChatMessage : public Packet
{
	SCPacketChatMessage(PID tid, const WCHAR* msg, size_t length);
	SCPacketChatMessage(PID tid, const WCHAR msg[]);

	WCHAR Caption[100];
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

enum class PACKET_TYPES : UCHAR
{
	NONE = 0,
	CS_SIGNIN,
	CS_SIGNOUT,
	CS_MOVE,
	CS_ATTACK_TARGET,
	CS_ATTACK_NONTARGET,
	CS_ATTACK_PLACE,
	CS_CHAT,

	SC_SIGNUP,
	SC_CREATE_PLAYER,
	SC_SIGNOUT,
	SC_CHAT,

	SC_APPEAR_OBJ,
	SC_DISAPPEAR_OBJ,
	SC_MOVE_OBJ,
	SC_STAT_OBJ,
};
