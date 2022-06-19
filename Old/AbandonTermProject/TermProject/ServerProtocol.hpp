constexpr int PORT_NUM = 4000;
constexpr int BUF_SIZE = 200;
constexpr int NAME_SIZE = 20;

constexpr int W_WIDTH = 2000;
constexpr int W_HEIGHT = 2000;

constexpr size_t MAX_USER = 10000;
constexpr size_t MAX_NPC = 200000;
constexpr size_t PLACE_USER_BEGIN = MAX_NPC;
constexpr size_t PLACE_NPC_BEGIN = 0;

enum DIRECTIONS : unsigned char
{
	NONE = 0,
	LEFT, RIGHT, UP, DOWN
};

enum CHAT_TYPES : unsigned char
{
	SAY = 1,		// 1:1 대화
	TELL,			// 시야 내 대화
	SHOUT_TO_ALL	// 전체 대화
};

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
