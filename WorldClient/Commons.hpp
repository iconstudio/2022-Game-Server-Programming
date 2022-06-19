#pragma once
#include "stdafx.hpp"

using PID = long long;

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

enum class ENTITY_TYPES : UCHAR
{
	NONE, NPC, QUEEST_NPC, MOB, BOSS, PLAYER
};

#pragma pack(push, 1)
class Packet
{
public:
	constexpr Packet(PACKET_TYPES type, UCHAR size, PID pid)
		: Type(type), Size(size), playerID(pid)
	{}

	constexpr Packet(PACKET_TYPES type, PID pid = 0)
		: Packet(type, sizeof(Packet), pid)
	{}

	constexpr virtual ~Packet() {}

	UCHAR Size;
	PACKET_TYPES Type;
	PID playerID;
};

/// <summary>
/// ������ �α��� �ǻ� �˸�
/// </summary>
struct CSPacketSignIn : public Packet
{
	CSPacketSignIn(const CHAR* nickname)
		: Packet(PACKET_TYPES::CS_SIGNIN, sizeof(CSPacketSignIn), 0)
		, Nickname()
	{
		strcpy_s(Nickname, nickname);
	}

	CHAR Nickname[30];
};

/// <summary>
/// �������� ������ �ǻ� �˸�
/// </summary>
struct CSPacketSignOut : public Packet
{
	CSPacketSignOut(PID pid)
		: Packet(PACKET_TYPES::CS_SIGNOUT, pid)
	{}
};

/// <summary>
/// ������ �����̴� ���� �˸�
/// </summary>
struct CSPacketMove : public Packet
{
	CSPacketMove(PID pid, UCHAR key)
		: Packet(PACKET_TYPES::CS_MOVE, sizeof(CSPacketMove), pid)
		, Key(key)
	{}

	const UCHAR Key;
};

/// <summary>
/// ��ȭ �޽���, ����� -1�̸� ��ο��� ������.
/// </summary>
struct CSPacketChatMessage : public Packet
{
	CSPacketChatMessage(PID fid, PID tid, const WCHAR* msg, size_t length)
		: Packet(PACKET_TYPES::CS_CHAT, sizeof(CSPacketChatMessage), fid)
		, Caption(), targetID(tid)
	{
		auto wstr = lstrcpyn(Caption, msg, int(length));
	}

	CSPacketChatMessage(PID fid, PID tid, const WCHAR msg[])
		: Packet(PACKET_TYPES::CS_CHAT, sizeof(CSPacketChatMessage), fid)
		, Caption()
	{
		lstrcpy(Caption, msg);
	}

	PID targetID;
	WCHAR Caption[100];
};

/// <summary>
/// ���� ������ �÷��̾�� ID �ο�, ���� ������ ��, �ִ� ������ �� �˸���
/// (ù ���� �ÿ��� ����)
/// </summary>
struct SCPacketSignUp : public Packet
{
	SCPacketSignUp(PID nid, UINT users = 0, UINT usersmax = 0)
		: Packet(PACKET_TYPES::SC_SIGNUP, sizeof(SCPacketSignUp), nid)
		, usersCurrent(users), usersMax(usersmax)
	{}

	UINT usersCurrent, usersMax;
};

/// <summary>
/// Ư�� �÷��̾��� ���� ���� ����
/// </summary>
struct SCPacketCreatePlayer : public Packet
{
	/// <param name="pid">�ٸ� �÷��̾��� ���� �ĺ���</param>
	/// <param name="nickname">����</param>
	SCPacketCreatePlayer(PID pid, const CHAR* nickname)
		: Packet(PACKET_TYPES::SC_CREATE_PLAYER, sizeof(SCPacketCreatePlayer), pid)
		, Nickname()
	{
		strcpy_s(Nickname, nickname);
	}

	CHAR Nickname[30];
};

/// <summary>
/// Ư�� ��ü�� �þ� ����
/// </summary>
struct SCPacketAppearCharacter : public Packet
{
	SCPacketAppearCharacter(PID cid, ENTITY_TYPES type, float cx, float cy)
		: Packet(PACKET_TYPES::SC_APPEAR_OBJ, sizeof(SCPacketAppearCharacter), cid)
		, myType(type), x(cx), y(cy)
	{}

	ENTITY_TYPES myType;
	float x, y;
};

/// <summary>
/// Ư�� ��ü�� ĳ���� �̵�
/// </summary>
struct SCPacketMoveCharacter : public Packet
{
	SCPacketMoveCharacter(PID cid, float nx, float ny)
		: Packet(PACKET_TYPES::SC_MOVE_OBJ, sizeof(SCPacketMoveCharacter), cid)
		, x(nx), y(ny)
	{}

	float x, y;
};

/// <summary>
/// Ư�� ��ü�� �þ� Ż��
/// </summary>
struct SCPacketDisppearCharacter : public Packet
{
	SCPacketDisppearCharacter(PID cid)
		: Packet(PACKET_TYPES::SC_DISAPPEAR_OBJ, cid)
	{}
};

/// <summary>
/// ��ȭ �޽���, ����� -1�̸� ��ο��� ������.
/// </summary>
struct SCPacketChatMessage : public Packet
{
	SCPacketChatMessage(PID tid, const WCHAR* msg, size_t length)
		: Packet(PACKET_TYPES::SC_CHAT, sizeof(SCPacketChatMessage), tid)
		, Caption()
	{
		auto wstr = lstrcpyn(Caption, msg, int(length));
	}

	SCPacketChatMessage(PID tid, const WCHAR msg[])
		: Packet(PACKET_TYPES::SC_CHAT, sizeof(SCPacketChatMessage), tid)
		, Caption()
	{
		lstrcpy(Caption, msg);
	}

	WCHAR Caption[100];
};

/// <summary>
/// �α��� ����
/// </summary>
struct SCPacketSignOut : public Packet
{
	SCPacketSignOut(PID pid, UINT users)
		: Packet(PACKET_TYPES::SC_SIGNOUT, sizeof(SCPacketSignOut), pid)
		, usersCurrent(users)
	{}

	UINT usersCurrent;
};
#pragma pack(pop)

constexpr COLORREF C_BLACK = RGB(0, 0, 0);
constexpr COLORREF C_WHITE = RGB(255, 255, 255);
constexpr COLORREF C_GREEN = RGB(0, 128, 0);
constexpr COLORREF C_GOLD = RGB(223, 130, 20);
constexpr int CLIENT_W = 800;
constexpr int CLIENT_H = 600;
constexpr float FRAME_W = 800.0f;
constexpr float FRAME_H = 600.0f;

constexpr USHORT PORT = 6000;
constexpr UINT PLAYERS_MAX_NUMBER = 10000;
constexpr UINT NPC_MAX_NUMBER = 10000;
constexpr UINT ENTITIES_MAX_NUMBER = PLAYERS_MAX_NUMBER + NPC_MAX_NUMBER;
constexpr UINT CLIENTS_ORDER_BEGIN = NPC_MAX_NUMBER;

constexpr PID NPC_ID_BEGIN = 0;
constexpr PID CLIENTS_ID_BEGIN = PID(NPC_MAX_NUMBER);

constexpr float CELL_SIZE = 32.0f;
constexpr float CELL_W = CELL_SIZE;
constexpr float CELL_H = CELL_SIZE;

constexpr int WORLD_CELLS_CNT_H = 2000;
constexpr int WORLD_CELLS_CNT_V = 2000;
constexpr float WORLD_W = CELL_W * WORLD_CELLS_CNT_H;
constexpr float WORLD_H = CELL_H * WORLD_CELLS_CNT_V;

constexpr int SIGHT_CELLS_CNT_H = 15;
constexpr int SIGHT_CELLS_CNT_V = 15;
constexpr int SIGHT_CELLS_RAD_H = SIGHT_CELLS_CNT_H / 2;
constexpr int SIGHT_CELLS_RAD_V = SIGHT_CELLS_CNT_H / 2;

constexpr float SIGHT_W = CELL_W * SIGHT_CELLS_CNT_H;
constexpr float SIGHT_H = CELL_H * SIGHT_CELLS_CNT_V;
constexpr float SIGHT_RAD_W = CELL_W * SIGHT_CELLS_RAD_H;
constexpr float SIGHT_RAD_H = CELL_H * SIGHT_CELLS_RAD_V;
