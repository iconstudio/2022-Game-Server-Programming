#pragma once

#pragma pack (push, 1)
struct Packet
{
	Packet(PACKET_TYPES type, PID id, unsigned char size)
		: myType(type), mySize(size), myID(id)
	{}

	const PACKET_TYPES myType;
	const unsigned char mySize;
	PID myID; // Object ID: 0 - 100만
};

// 로그인 시도
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

// 캐릭터 이동
struct CS_MOVE_PACKET : public Packet
{
	CS_MOVE_PACKET(PACKET_TYPES type, PID id, unsigned char size, DIRECTIONS dir, unsigned int time)
		: Packet(type, id, size)
		, direction(dir), client_time(time)
	{}

	DIRECTIONS direction;
	unsigned client_time;
};

// 공격 시도
struct CS_ATTACK_PACKET : public Packet
{};

// 채팅 메시지 보냄
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
