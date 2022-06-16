constexpr int PORT_NUM = 4000;
constexpr int BUF_SIZE = 200;
constexpr int NAME_SIZE = 20;

constexpr int W_WIDTH = 2000;
constexpr int W_HEIGHT = 2000;

constexpr int MAX_USER = 1000;
constexpr int NUM_NPC = 200000;

// Packet ID
constexpr char CS_LOGIN = 0;
/*
	ID
	���� : Ư�� �Ÿ� ��
	��� : Ư�� �Ÿ� ��
*/
constexpr char CS_MOVE = 20;
constexpr char CS_CHAT = 21;
/*
	����
	��ǥ ID
	�޽��� ����
*/

constexpr char SC_LOGIN_INFO = 2;
/*
	ID
	�α��� ���� ����
	������ ��, �ִ� ������ ��
	[���� ����]
*/

constexpr char SC_ADD_PLAYER = 3; // SC_ADD_OBJECT. �þ߿� �߰��ϴ� ��!!!
/*
	 ID (�߰��� ��ü�� ID)
	 ����
	 �̸�
	 [����]
	 [Īȣ]
	 [ü��, �ִ� ü�� �� ����]

	 // ������ �ܼ��̶� ������ ������ �� �־�� �Ѵ�!!!!!
*/
constexpr char SC_REMOVE_PLAYER = 4; // SC_REMOVE_OBJECT, �þ߿��� �����ϴ� ��!!!

constexpr char SC_MOVE_PLAYER = 5; // SC_MOVE_OBJECT

constexpr char SC_OBJECT_ACTION = 6; // ��ü�� � �ൿ�� ���� �� �˸�

constexpr char SC_ATTACK_OBJECT = 7; // ��� ������ ����

constexpr char SC_STATE_CHANGED = 9; // ��ü�� ���� ��ȭ �˸�
/*
	ADD���� ���� �͵��� ���߷��� �ٽ� ������.
*/

constexpr char SC_REMOVE_CLIENT = 10; // �÷��̾��� ���� ���� �˸�
/*
	ID
	ü��, �ִ� ü��
	����
	�̵� �ӵ�
	��ǥ

*/

constexpr char SC_CHAT = 90; // ä��
/*
	ID // �÷��̾����׸� ����
	����
	/
		����� = 0
		�ý��� = 1
		1:1 ��ȭ = 2
		��ü ��ȭ = 3
		��Ƽ ��ȭ = 4
	/
	
	�޽��� ����
*/


#pragma pack (push, 1)
struct CS_LOGIN_PACKET {
	unsigned char size;
	char	type;
	char	name[NAME_SIZE];
};

struct CS_MOVE_PACKET {
	unsigned char size;
	char	type;
	char	direction;  // 0 : UP, 1 : DOWN, 2 : LEFT, 3 : RIGHT
	unsigned  client_time;
};

struct SC_LOGIN_INFO_PACKET {
	unsigned char size;
	char	type;
	short	id;
	short	x, y;
};

struct SC_ADD_PLAYER_PACKET {
	unsigned char size;
	char	type;
	short	id;
	short	x, y;
	char	name[NAME_SIZE];
};

struct SC_REMOVE_PLAYER_PACKET {
	unsigned char size;
	char	type;
	short	id;
};

struct SC_MOVE_PLAYER_PACKET {
	unsigned char size;
	char	type;
	short	id;
	short	x, y;
	unsigned int client_time;
};

#pragma pack (pop)