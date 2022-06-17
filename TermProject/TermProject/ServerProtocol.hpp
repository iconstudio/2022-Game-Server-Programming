constexpr int PORT_NUM = 4000;
constexpr int BUF_SIZE = 200;
constexpr int NAME_SIZE = 20;

constexpr int W_WIDTH = 2000;
constexpr int W_HEIGHT = 2000;

constexpr size_t MAX_USER = 10000;
constexpr size_t MAX_NPC = 200000;
constexpr size_t PLACE_USER_BEGIN = MAX_NPC;
constexpr size_t PLACE_NPC_BEGIN = 0;

enum PACKET_TYPES : unsigned char
{
	CS_LOGIN = 1,
	CS_MOVE,
	CS_ATTACK,
	CS_CHAT,

	SC_LOGIN_OK,
	SC_LOGIN_FAIL,
	SC_ADD_OBJECT,
	SC_STAT_CHANGE,
	SC_REMOVE_OBJECT,
	SC_MOVE_OBJECT,
	SC_CHAT,
};

enum CHAT_TYPES : unsigned char
{
	SAY = 1,		// 1:1 대화
	TELL,			// 시야 내 대화
	SHOUT_TO_ALL	// 전체 대화
};

#pragma pack (push, 1)
struct Packet
{
	unsigned char mySize;
	PACKET_TYPES myType;

	// Object ID: 0 - 100만
	PID id;
};

struct CS_LOGIN_PACKET : public Packet
{
	char name[NAME_SIZE];
};

struct CS_MOVE_PACKET : public Packet
{
	char direction;  // 0 : UP, 1 : DOWN, 2 : LEFT, 3 : RIGHT
	unsigned client_time;
};

struct CS_ATTACK_PACKET : public Packet
{};

struct CS_CHAT_PACKET : public Packet
{
	CHAT_TYPES chat_type;

	// 1:1 대화일 때만 id 설정
	PID target_id;
	char mess[BUF_SIZE];
};

struct SC_LOGIN_OK_PACKET : public Packet
{
	CLASS_TYPES myClass;

	short level;
	int	  exp;

	int   hp, hpmax;
	int x, y;
};

struct SC_LOGIN_FAIL_PACKET : public Packet
{
	int reason;		// 0 : Invalid Name  (특수문자, 공백 제외)
						// 1 : Name Already Playing
						// 2 : Server Full
};

struct SC_ADD_OBJECT_PACKET : public Packet
{
	int x, y;
	short race;	// 종족 : 인간, 엘프, 드워프, 오크, 드래곤
				// 클라이언트에서 종족별로 별도의 그래픽 표현
				// 추가적으로 성별이나, 직업을 추가할 수 있다.
	char	name[NAME_SIZE];
	short	level;
	int		hp, hpmax;
};

struct SC_REMOVE_OBJECT_PACKET : public Packet
{};

struct SC_MOVE_OBJECT_PACKET : public Packet
{
	int x, y;
	unsigned int client_time;
};

struct SC_CHAT_PACKET : public Packet
{
							// -1 : System Message
	char	chat_type;		// 0 : 일반 채팅 (say)
							// 1 : 1:1 통신 (tell)
							// 2 : 방송 (shout)
							// 3 : 공지
	char	mess[BUF_SIZE];
};

struct SC_STAT_CHANGE_PACKET : public Packet
{
	short	level;
	int		exp;
	int		hp, hpmax;
};

#pragma pack (pop)