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
	계정 : 특문 거를 것
	비번 : 특문 거를 것
*/
constexpr char CS_MOVE = 20;
constexpr char CS_CHAT = 21;
/*
	종류
	목표 ID
	메시지 내용
*/

constexpr char SC_LOGIN_INFO = 2;
/*
	ID
	로그인 성공 여부
	접속자 수, 최대 접속자 수
	[실패 이유]
*/

constexpr char SC_ADD_PLAYER = 3; // SC_ADD_OBJECT. 시야에 추가하는 것!!!
/*
	 ID (추가할 객체의 ID)
	 종류
	 이름
	 [종족]
	 [칭호]
	 [체력, 최대 체력 등 스펙]

	 // 심지어 콘솔이라도 종류를 구분할 수 있어야 한다!!!!!
*/
constexpr char SC_REMOVE_PLAYER = 4; // SC_REMOVE_OBJECT, 시야에서 제거하는 것!!!

constexpr char SC_MOVE_PLAYER = 5; // SC_MOVE_OBJECT

constexpr char SC_OBJECT_ACTION = 6; // 객체가 어떤 행동을 취할 때 알림

constexpr char SC_ATTACK_OBJECT = 7; // 대상 지정형 공격

constexpr char SC_STATE_CHANGED = 9; // 객체의 상태 변화 알림
/*
	ADD에서 보낸 것들을 간추려서 다시 보낸다.
*/

constexpr char SC_REMOVE_CLIENT = 10; // 플레이어의 접속 종료 알림
/*
	ID
	체력, 최대 체력
	방어력
	이동 속도
	좌표

*/

constexpr char SC_CHAT = 90; // 채팅
/*
	ID // 플레이어한테만 전송
	종류
	/
		디버그 = 0
		시스템 = 1
		1:1 대화 = 2
		전체 대화 = 3
		파티 대화 = 4
	/
	
	메시지 내용
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