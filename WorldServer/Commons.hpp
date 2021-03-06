#pragma once
#include <string.h>
#include <WinBase.h>

using PID = long long;

enum class PACKET_TYPES : unsigned char
{
	NONE = 0,
	CS_SIGNIN = 1,
	CS_SIGNOUT,
	CS_MOVE,
	CS_ATTACK_TARGET,
	CS_ATTACK_NONTARGET,
	CS_ATTACK_PLACE,
	CS_SKILL_0,
	CS_SKILL_1,
	CS_SKILL_2,
	CS_CHAT,

	SC_SIGNUP = 50,
	SC_SIGNOUT,
	SC_CREATE_PLAYER,
	SC_SIGNIN_FAILED,
	SC_CHAT,

	SC_APPEAR_OBJ = 150,
	SC_DISAPPEAR_OBJ,
	SC_MOVE_OBJ,
	SC_STAT_OBJ,
	SC_UPDATE_PLAYER,
};

enum class LOGIN_ERROR_TYPES : unsigned char
{
	NONE = 0,
	SUCEED,
	USERS_LIMITED,
	BANNED,
	NETWORK_ERROR
};

enum class CHAT_MSG_TYPES : unsigned char
{
	NONE = 0,
	PLAIN, ONE_ON_ONE, SHOUT,
	SYSTEM,
};

// 객체의 범주
enum class ENTITY_CATEGORY : unsigned char
{
	NONE = 0
	, NPC, MOB, BOSS, PLAYER
};

// 객체의 하위 종류
enum class ENTITY_TYPES : unsigned int
{
	NONE = 0,
	PLAYER_WARRIOR = 1,
	PLAYER_ARCHER,
	PLAYER_MAGICIAN,
	NPC_SHOPKEEPER = 1000,
	NPC_GUARD,
	MOB_SLIME_1 = 10000,
	MOB_DEMON,
	MOB_SKELETON,
	MOB_SKELETON_KING
};

enum class OVERLAP_OPS : unsigned char
{
	NONE = 0,
	LISTEN,
	ACCEPT,
	RECV,
	SEND,
	ENTITY_MOVE = 100,
	ENTITY_ATTACK,
	PLAYER_ATTACK,
	ENTITY_ACTION,
	ENTITY_HEAL,
	ENTITY_DEAD,
};

enum class MOVE_TYPES : unsigned char
{
	NONE = 0,
	LEFT, UP, RIGHT, DOWN
};

#pragma pack(push, 1)
class Packet
{
public:
	constexpr Packet(PACKET_TYPES type, unsigned char size, PID pid)
		: Type(type), Size(size), playerID(pid)
	{}

	constexpr Packet(PACKET_TYPES type, PID pid = 0)
		: Packet(type, sizeof(Packet), pid)
	{}

	constexpr virtual ~Packet() {}

	unsigned char Size;
	PACKET_TYPES Type;
	PID playerID;
};

/// <summary>
/// 서버에 로그인 의사 알림
/// </summary>
struct CSPacketSignIn : public Packet
{
	CSPacketSignIn(const char* nickname)
		: Packet(PACKET_TYPES::CS_SIGNIN, sizeof(CSPacketSignIn), 0)
		, Nickname()
	{
		strcpy_s(Nickname, nickname);
	}

	char Nickname[30];
};

/// <summary>
/// 서버에서 나가는 의사 알림
/// </summary>
struct CSPacketSignOut : public Packet
{
	CSPacketSignOut(PID pid)
		: Packet(PACKET_TYPES::CS_SIGNOUT, pid)
	{}
};

/// <summary>
/// 서버에 움직이는 동작 알림
/// </summary>
struct CSPacketMove : public Packet
{
	CSPacketMove(PID pid, MOVE_TYPES dir)
		: Packet(PACKET_TYPES::CS_MOVE, sizeof(CSPacketMove), pid)
		, myDirection(dir)
	{}

	const MOVE_TYPES myDirection;
	unsigned move_time = 0;
};

/// <summary>
/// 서버에 기본 공격의 동작 알림
/// </summary>
struct CSPacketAttack : public Packet
{
	CSPacketAttack(PID pid, MOVE_TYPES dir)
		: Packet(PACKET_TYPES::CS_ATTACK_NONTARGET, sizeof(CSPacketAttack), pid)
		, attackDirection(dir)
	{}

	const MOVE_TYPES attackDirection;
};

/// <summary>
/// 서버에 대상 지정 공격의 동작 알림
/// </summary>
struct CSPacketAttackTarget : public Packet
{
	CSPacketAttackTarget(PID pid, PID target_id, UCHAR attack_type)
		: Packet(PACKET_TYPES::CS_ATTACK_TARGET, sizeof(CSPacketAttackTarget), pid)
		, attackTarget(target_id), attackType(attack_type)
	{}

	const PID attackTarget;
	const unsigned char attackType;
};

/// <summary>
/// 대화 메시지, 대상이 -1이면 모두에게 보낸다.
/// </summary>
struct CSPacketChatMessage : public Packet
{
	CSPacketChatMessage(PID fid, PID tid, CHAT_MSG_TYPES type, const wchar_t* msg, size_t length)
		: Packet(PACKET_TYPES::CS_CHAT, sizeof(CSPacketChatMessage), fid)
		, myType(type)
		, Caption()
		, targetID(tid)
	{
		auto wstr = lstrcpyn(Caption, msg, int(length));
	}

	CSPacketChatMessage(PID fid, PID tid, CHAT_MSG_TYPES type, const wchar_t msg[])
		: Packet(PACKET_TYPES::CS_CHAT, sizeof(CSPacketChatMessage), fid)
		, myType(type)
		, Caption()
		, targetID(0)
	{
		lstrcpy(Caption, msg);
	}

	const CHAT_MSG_TYPES myType;
	PID targetID = -1;
	wchar_t Caption[100];
};

/// <summary>
/// 새로 접속한 플레이어에게 ID 부여, 현재 동접자 수, 최대 동접자 수 알리기
/// (첫 접속 시에만 실행)
/// </summary>
struct SCPacketSignUp : public Packet
{
	SCPacketSignUp(PID nid, unsigned int users = 0, unsigned int usersmax = 0)
		: Packet(PACKET_TYPES::SC_SIGNUP, sizeof(SCPacketSignUp), nid)
		, usersCurrent(users), usersMax(usersmax)
	{}

	unsigned int usersCurrent, usersMax;
};

/// <summary>
/// 특정 플레이어의 접속 종료 알림
/// </summary>
struct SCPacketSignOut : public Packet
{
	SCPacketSignOut(PID pid)
		: Packet(PACKET_TYPES::SC_SIGNOUT, sizeof(SCPacketSignUp), pid)
	{}
};

/// <summary>
/// 특정 플레이어의 로컬 세션 생성
/// </summary>
struct SCPacketCreatePlayer : public Packet
{
	SCPacketCreatePlayer(PID pid, const char* nickname)
		: Packet(PACKET_TYPES::SC_CREATE_PLAYER, sizeof(SCPacketCreatePlayer), pid)
		, Nickname()
	{
		strcpy_s(Nickname, nickname);
	}

	char Nickname[30];
};

/// <summary>
/// 특정 개체의 시야 진입
/// </summary>
struct SCPacketAppearEntity : public Packet
{
	SCPacketAppearEntity(PID cid)
		: Packet(PACKET_TYPES::SC_APPEAR_OBJ, sizeof(SCPacketAppearEntity), cid)
		, myCategory(), myType()
		, x(), y()
	{}

	SCPacketAppearEntity(PID cid, ENTITY_CATEGORY category, ENTITY_TYPES type, float cx, float cy)
		: Packet(PACKET_TYPES::SC_APPEAR_OBJ, sizeof(SCPacketAppearEntity), cid)
		, myCategory(category), myType(type)
		, x(cx), y(cy)
	{}

	ENTITY_CATEGORY myCategory;
	ENTITY_TYPES myType;

	MOVE_TYPES dir = MOVE_TYPES::NONE;
	int level = 0;
	int hp = 1, maxhp = 1;
	int mp = 0, maxmp = 0;
	int amour = 0;
	float x, y;
};

/// <summary>
/// 특정 개체의 캐릭터 이동
/// </summary>
struct SCPacketMoveCharacter : public Packet
{
	SCPacketMoveCharacter(PID cid, float nx, float ny, MOVE_TYPES dir)
		: Packet(PACKET_TYPES::SC_MOVE_OBJ, sizeof(SCPacketMoveCharacter), cid)
		, x(nx), y(ny), placeDirection(dir)
	{}

	const float x, y;
	const MOVE_TYPES placeDirection;
	unsigned move_time = 0;
};

/// <summary>
/// 특정 개체의 시야 탈출
/// </summary>
struct SCPacketDisppearCharacter : public Packet
{
	SCPacketDisppearCharacter(PID cid)
		: Packet(PACKET_TYPES::SC_DISAPPEAR_OBJ, cid)
	{}
};

/// <summary>
/// 대화 메시지
/// </summary>
struct SCPacketChatMessage : public Packet
{
	SCPacketChatMessage(PID tid, CHAT_MSG_TYPES type, const WCHAR* msg, size_t length)
		: Packet(PACKET_TYPES::SC_CHAT, sizeof(SCPacketChatMessage), tid)
		, myType(type)
		, Caption()
	{
		auto wstr = lstrcpyn(Caption, msg, int(length));
	}

	SCPacketChatMessage(PID tid, CHAT_MSG_TYPES type, const WCHAR msg[])
		: Packet(PACKET_TYPES::SC_CHAT, sizeof(SCPacketChatMessage), tid)
		, myType(type)
		, Caption()
	{
		lstrcpy(Caption, msg);
	}

	const CHAT_MSG_TYPES myType;
	wchar_t Caption[100];
};

/// <summary>
/// 로그인 실패
/// </summary>
struct SCPacketSignInFailed : public Packet
{
	SCPacketSignInFailed(PID pid, LOGIN_ERROR_TYPES reason, UINT users)
		: Packet(PACKET_TYPES::SC_SIGNIN_FAILED, sizeof(SCPacketSignInFailed), pid)
		, myReason(reason)
		, usersCurrent(users)
	{}

	const LOGIN_ERROR_TYPES myReason;
	const unsigned int usersCurrent;
};
#pragma pack(pop)

constexpr int CLIENT_W = 800;
constexpr int CLIENT_H = 600;
constexpr float FRAME_W = float(CLIENT_W);
constexpr float FRAME_H = float(CLIENT_H);

constexpr unsigned short PORT = 6000;
constexpr size_t BUFFSZ = 256;

constexpr unsigned int USERS_MAX = 10000;
constexpr unsigned int NPCS_MAX = 200000;
constexpr unsigned int ENTITIES_MAX_NUMBER = USERS_MAX + NPCS_MAX;

constexpr unsigned int NPC_ID_BEGIN = 0;
constexpr unsigned int NPC_ID_END = NPCS_MAX;
constexpr unsigned int USERS_ID_BEGIN = NPC_ID_BEGIN + NPCS_MAX;
constexpr unsigned int USERS_ID_END = USERS_ID_BEGIN + USERS_MAX;

constexpr PID CLIENTS_ORDER_BEGIN = NPCS_MAX;

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

constexpr bool IsPlayer(PID id)
{
	return CLIENTS_ORDER_BEGIN <= id;
}

constexpr bool IsNonPlayer(PID id)
{
	return id < CLIENTS_ORDER_BEGIN;
}
