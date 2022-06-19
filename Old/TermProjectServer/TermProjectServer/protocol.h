constexpr unsigned short PORT_NUM = 8000;
constexpr int BUF_SIZE = 200;
constexpr int NAME_SIZE = 20;

constexpr int W_WIDTH = 2000;
constexpr int W_HEIGHT = 2000;

using PID = long long;
constexpr PID MAX_USER = 10000;
constexpr PID MAX_NPC = 200000;
constexpr PID ORDER_BEGIN_NPC = 0;
constexpr PID ORDER_BEGIN_USER = MAX_NPC;
constexpr PID ORDER_END_NPC = ORDER_BEGIN_NPC + MAX_NPC;
constexpr PID ORDER_END_USER = ORDER_BEGIN_USER + MAX_USER;
constexpr PID ORDER_END_ALL = ORDER_END_USER;

class Session;
class GameObject;
enum PACKET_TYPES : unsigned char;
enum class PLAYER_CATEGORY : unsigned char;
enum class RACE_TYPES : unsigned char;
enum class MOVE_TYPES : unsigned char;

enum PACKET_TYPES : unsigned char
{
	NONE = 0,

	CS_LOGIN,
	CS_MOVE,
	CS_ATTACK,
	CS_ATTACK_NONTARGET,
	CS_SKILL_1,
	CS_SKILL_2,
	CS_CHAT,

	SC_LOGIN_OK,
	SC_LOGIN_FAIL,
	SC_ADD_OBJECT,
	SC_REMOVE_OBJECT,
	SC_MOVE_OBJECT,
	SC_JUMP_OBJECT = SC_MOVE_OBJECT,
	SC_CHANGE_OBJECT_STAT,
	SC_CHAT,
};

enum class PLAYER_CATEGORY : unsigned char
{
	NONE = 0,
	NPC_FRIENDLY, // 우호 NPC
	NPC_NEUTRAL, // 중립 NPC (비선공)
	NPC_HOSTILE, // 적대 NPC (몹)
	HUMAN,
};

enum class RACE_TYPES : unsigned char
{
	NONE = 0,
	NPC,
	HUMAN
};

enum class MOVE_TYPES : unsigned char
{
	NONE = 0,
	LEFT, RIGHT, UP, DOWN
};

constexpr bool IsPlayer(const PID id)
{
	return (MAX_NPC <= id);
}

constexpr bool IsNPC(const PID id)
{
	return (id < MAX_NPC);
}

template<size_t length>
constexpr void string_copy_safe(char(&dest)[length], const char* src, size_t src_length)
{
	for (size_t i = 0; i < length; i++)
	{
		if (src_length <= i) break;

		const char now = *(src + i);

		if ('0' == now) break;

		dest[i] = now;
	}
};

#pragma pack (push, 1)
struct Packet
{
	constexpr Packet(PACKET_TYPES type, unsigned char sz)
		: myType(type), mySize(sz)
	{}

	constexpr Packet(PACKET_TYPES type)
		: myType(type), mySize(sizeof(Packet))
	{}

	constexpr virtual ~Packet() {}

	unsigned char mySize;
	const PACKET_TYPES myType;
};

struct CS_LOGIN_PACKET : public Packet
{
	constexpr CS_LOGIN_PACKET(const string& nickname)
		: Packet(PACKET_TYPES::CS_LOGIN, sizeof(CS_LOGIN_PACKET))
		, myNickname()
	{
		string_copy_safe(myNickname, nickname.c_str(), nickname.size());
	}

	char myNickname[NAME_SIZE];
};

struct CS_MOVE_PACKET : public Packet
{
	constexpr CS_MOVE_PACKET(MOVE_TYPES dir)
		: Packet(PACKET_TYPES::CS_MOVE, sizeof(CS_MOVE_PACKET))
		, direction(dir)
		, client_time(0)
	{}
	
	const MOVE_TYPES direction;
	unsigned int client_time;
};

struct CS_ATTACK_PACKET : public Packet
{
	constexpr CS_ATTACK_PACKET(PID attacker, PID target, float dmg, float ax, float ay)
		: Packet(PACKET_TYPES::CS_ATTACK, sizeof(CS_ATTACK_PACKET))
		, attackerID(attacker), targetID(target)
		, destDamage(dmg)
		, destAddX(ax), destAddY(ay)
	{}

	const PID attackerID;
	const PID targetID;

	const float destDamage;
	// 플레이어 기준 (격자)
	const float destAddX, destAddY;
};

struct CS_ATTACK_NONTARGET_PACKET : public Packet
{
	constexpr CS_ATTACK_NONTARGET_PACKET(PID attacker, float dmg, float dx, float dy, float range)
		: Packet(PACKET_TYPES::CS_ATTACK_NONTARGET, sizeof(CS_ATTACK_NONTARGET_PACKET))
		, attackerID(attacker)
		, destDamage(dmg)
		, destX(dx), destY(dy), destRange(range)
	{}

	const PID attackerID;

	const float destDamage;
	const float destX, destY;
	const float destRange;
};

struct CS_CHAT_PACKET : public Packet
{
	constexpr CS_CHAT_PACKET(PID talker, char type, const string& msg, PID target = PID(-1))
		: Packet(PACKET_TYPES::CS_CHAT, sizeof(CS_CHAT_PACKET))
		, talkerID(talker)
		, myType(type), myMsg()
		, targetID(target)
	{
		string_copy_safe(myMsg, msg.c_str(), msg.size());
	}

	// 시스템 메시지는 PID(-1)
	const PID talkerID;
	// 전체 대화에서는 사용 안함
	PID targetID;

	// 1 : say,  2 : tell, 3 : shout
	const char myType;
	char myMsg[BUF_SIZE];
};

struct SC_LOGIN_OK_PACKET : public Packet
{
	constexpr SC_LOGIN_OK_PACKET(PID id, size_t users_count, size_t users_limit)
		: Packet(PACKET_TYPES::SC_LOGIN_OK, sizeof(SC_LOGIN_OK_PACKET))
		, myID(id)
		, usersCount(users_count), usersLimit(users_limit)
		, level(0), exp(0)
		, race(0)
		, hp(0), hpmax(0), armour(0)
		, x(0.0f), y(0.0f)
	{}

	const PID myID;
	const size_t usersCount, usersLimit;

	short level;
	int exp;
	short race;
	int hp, hpmax;
	int armour;
	float x, y;
};

struct SC_LOGIN_FAIL_PACKET : public Packet
{
	constexpr SC_LOGIN_FAIL_PACKET(int reason)
		: Packet(PACKET_TYPES::SC_LOGIN_FAIL, sizeof(SC_LOGIN_FAIL_PACKET))
		, myReason(reason)
	{}

	// 0 : Invalid Name  (특수문자, 공백 제외)
	// 1 : Name Already Playing
	// 2 : Server Full
	const int myReason;
};

struct SC_ADD_OBJECT_PACKET : public Packet
{
	constexpr SC_ADD_OBJECT_PACKET(PID id, PLAYER_CATEGORY type, float cx, float cy)
		: Packet(PACKET_TYPES::SC_ADD_OBJECT, sizeof(SC_ADD_OBJECT_PACKET))
		, myID(id), playerType(type)
		, x(cx), y(cy)
		, myNickname()
	{}

	constexpr SC_ADD_OBJECT_PACKET(PID id, PLAYER_CATEGORY type, const string& nickname, float cx = 0.0f, float cy = 0.0f)
		: Packet(PACKET_TYPES::SC_ADD_OBJECT, sizeof(SC_ADD_OBJECT_PACKET))
		, myID(id), playerType(type)
		, x(0.0f), y(0.0f)
	{
		string_copy_safe(myNickname, nickname.c_str(), nickname.size());
	}

	const PID myID;
	const PLAYER_CATEGORY playerType;

	char myNickname[NAME_SIZE];

	// 종족 : 인간, 엘프, 드워프, 오크, 드래곤
	// 클라이언트에서 종족별로 별도의 그래픽 표현
	// 추가적으로 성별이나, 직업을 추가할 수 있다.
	short race = 0;

	short level = 0;
	const float x, y;
	int hp = 1, hpmax = 1;
	int armour = 0;
};

struct SC_MOVE_OBJECT_PACKET : public Packet
{
	constexpr SC_MOVE_OBJECT_PACKET(PID who, float mx, float my)
		: Packet(PACKET_TYPES::SC_MOVE_OBJECT, sizeof(SC_MOVE_OBJECT_PACKET))
		, myID(who)
		, x(mx), y(my)
		, client_time(0)
	{}

	const PID myID;
	const float x, y;

	unsigned int client_time;
};

struct SC_REMOVE_OBJECT_PACKET : public Packet
{
	constexpr SC_REMOVE_OBJECT_PACKET(PID who)
		: Packet(PACKET_TYPES::SC_REMOVE_OBJECT, sizeof(SC_REMOVE_OBJECT_PACKET))
		, targetID(who)
	{}

	const PID targetID;
};

struct SC_CHAT_PACKET : public Packet
{
	PID		id;				// 0 - 100만 : Object ID
							// -1 : System Message
	char	chat_type;		// 0 : 일반 채팅 (say)
							// 1 : 1:1 통신 (tell)
							// 2 : 방송 (shout)
							// 3 : 공지
	char	mess[BUF_SIZE];
};

struct SC_STAT_CHANGE_PACKET : public Packet
{
	PID		id;
	short	level;
	int		exp;
	int		hp, hpmax;
};

#pragma pack (pop)