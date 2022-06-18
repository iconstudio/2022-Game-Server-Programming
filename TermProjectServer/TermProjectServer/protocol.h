constexpr int PORT_NUM = 4000;
constexpr int BUF_SIZE = 200;
constexpr int NAME_SIZE = 20;

constexpr int W_WIDTH = 2000;
constexpr int W_HEIGHT = 2000;

constexpr int MAX_USER = 10000;
constexpr int NUM_NPC = 200000;

// Packet ID
constexpr char CS_LOGIN = 0;
constexpr char CS_MOVE = 1;
constexpr char CS_ATTACK = 2;
constexpr char CS_CHAT = 3;

constexpr char SC_LOGIN_OK = 11;
constexpr char SC_LOGIN_FAIL = 12;
constexpr char SC_ADD_OBJECT = 13;
constexpr char SC_REMOVE_OBJECT = 14;
constexpr char SC_MOVE_OBJECT = 15;
constexpr char SC_CHAT = 16;
constexpr char SC_STAT_CHANGE = 17;


#pragma pack (push, 1)
struct CS_LOGIN_PACKET
{
	unsigned char size;
	char	type;
	char	name[NAME_SIZE];
};

struct CS_MOVE_PACKET
{
	unsigned char size;
	char	type;
	char	direction;  // 0 : UP, 1 : DOWN, 2 : LEFT, 3 : RIGHT
	unsigned  client_time;
};

struct CS_ATTACK_PACKET
{
	unsigned char size;
	char	type;
};

struct CS_CHAT_PACKET
{
	unsigned char size;
	char	type;
	int		target_id;
	char	chat_type;			// 1 : say,  2 : tell, 3 : shout
	char	mess[BUF_SIZE];
};

struct SC_LOGIN_OK_PACKET
{
	unsigned char size;
	char	type;
	int	id;
	short race;
	short x, y;
	short level;
	int	  exp;
	int   hp, hpmax;
};

struct SC_LOGIN_FAIL_PACKET
{
	unsigned char size;
	char	type;
	int		reason;				// 0 : Invalid Name  (Ư������, ���� ����)
								// 1 : Name Already Playing
								// 2 : Server Full
};

struct SC_ADD_OBJECT_PACKET
{
	unsigned char size;
	char	type;
	int		id;
	short	x, y;
	short	race;			// ���� : �ΰ�, ����, �����, ��ũ, �巡��
							// Ŭ���̾�Ʈ���� �������� ������ �׷��� ǥ��
							// �߰������� �����̳�, ������ �߰��� �� �ִ�.
	char	name[NAME_SIZE];
	short	level;
	int		hp, hpmax;
};

struct SC_REMOVE_OBJECT_PACKET
{
	unsigned char size;
	char	type;
	int	id;
};

struct SC_MOVE_OBJECT_PACKET
{
	unsigned char size;
	char	type;
	int	id;
	short	x, y;
	unsigned int client_time;
};

struct SC_CHAT_PACKET
{
	unsigned char size;
	char	type;
	int		id;				// 0 - 100�� : Object ID
							// -1 : System Message
	char	chat_type;		// 0 : �Ϲ� ä�� (say)
							// 1 : 1:1 ��� (tell)
							// 2 : ��� (shout)
							// 3 : ����
	char	mess[BUF_SIZE];
};

struct SC_STAT_CHANGE_PACKET
{
	unsigned char size;
	char	type;
	int		id;
	short	level;
	int		exp;
	int		hp, hpmax;
};

#pragma pack (pop)