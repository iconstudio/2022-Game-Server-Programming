#pragma once

#pragma pack (push, 1)
struct Packet
{
	Packet(PACKET_TYPES type, PID id, unsigned char size)
		: myType(type), mySize(size), myID(id)
	{}

	const PACKET_TYPES myType;
	const unsigned char mySize;
	PID myID; // Object ID: 0 - 100��
};

// �α��� �õ�
struct CS_LOGIN_PACKET : public Packet
{
	CS_LOGIN_PACKET(PACKET_TYPES type, PID id, unsigned char size, const char* name)
		: Packet(type, id, size)
		, myNickname()
	{
		strcpy_s(myNickname, name);
	}

	CS_LOGIN_PACKET(Packet&& packet, const char* name)
		: Packet(std::forward<Packet>(packet))
		, myNickname()
	{
		strcpy_s(myNickname, name);
	}

	char myNickname[NAME_SIZE];
};

// ĳ���� �̵�
struct CS_MOVE_PACKET : public Packet
{
	CS_MOVE_PACKET(PACKET_TYPES type, PID id, unsigned char size, DIRECTIONS dir, unsigned int time)
		: Packet(type, id, size)
		, direction(dir), client_time(time)
	{}

	DIRECTIONS direction;
	unsigned client_time;
};

// ���� �õ�
struct CS_ATTACK_PACKET : public Packet
{};

// ä�� �޽��� ����
struct CS_CHAT_PACKET : public Packet
{
	CHAT_TYPES chat_type;

	// 1:1 ��ȭ�� ���� id ����
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
	int reason;		// 0 : Invalid Name  (Ư������, ���� ����)
						// 1 : Name Already Playing
						// 2 : Server Full
};

struct SC_ADD_OBJECT_PACKET : public Packet
{
	int x, y;
	short race;	// ���� : �ΰ�, ����, �����, ��ũ, �巡��
				// Ŭ���̾�Ʈ���� �������� ������ �׷��� ǥ��
				// �߰������� �����̳�, ������ �߰��� �� �ִ�.
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
	char	chat_type;		// 0 : �Ϲ� ä�� (say)
							// 1 : 1:1 ��� (tell)
							// 2 : ��� (shout)
							// 3 : ����
	char	mess[BUF_SIZE];
};

struct SC_STAT_CHANGE_PACKET : public Packet
{
	short	level;
	int		exp;
	int		hp, hpmax;
};
#pragma pack (pop)
