#pragma once
#include "Packet.hpp"

#pragma pack(push, 1)
/// <summary>
/// 서버에 로그인 의사 알림
/// </summary>
struct CSPacketSignIn : public Packet
{
	CSPacketSignIn(const CHAR* nickname);

	CHAR Nickname[30];
};

/// <summary>
/// 서버에서 나가는 의사 알림
/// </summary>
struct CSPacketSignOut : public Packet
{
	CSPacketSignOut(PID pid);
};

/// <summary>
/// 서버에 입력한 키 알림
/// </summary>
struct CSPacketMove : public Packet
{
	CSPacketMove(PID pid, UCHAR key);

	const UCHAR Key;
};

/// <summary>
/// 대화 메시지, 대상이 -1이면 모두에게 보낸다.
/// </summary>
struct CSPacketChatMessage : public Packet
{
	CSPacketChatMessage(PID fid, PID tid, const WCHAR* msg, size_t length);
	CSPacketChatMessage(PID fid, PID tid, const WCHAR msg[]);

	PID targetID;
	WCHAR Caption[100];
};

/// <summary>
/// 새로 접속한 플레이어에게 ID 부여, 현재 동접자 수, 최대 동접자 수 알리기
/// (첫 접속 시에만 실행)
/// </summary>
struct SCPacketSignUp : public Packet
{
	SCPacketSignUp(PID nid, UINT users = 0, UINT usersmax = 0);

	UINT usersCurrent, usersMax;
};

/// <summary>
/// 특정 플레이어의 로컬 세션 생성
/// </summary>
struct SCPacketCreatePlayer : public Packet
{
	/// <param name="pid">다른 플레이어의 고유 식별자</param>
	/// <param name="nickname">별명</param>
	SCPacketCreatePlayer(PID pid, const CHAR* nickname);

	CHAR Nickname[30];
};

/// <summary>
/// 특정 개체의 시야 진입
/// </summary>
struct SCPacketAppearCharacter : public Packet
{
	SCPacketAppearCharacter(PID cid, ENTITY_TYPES type, float cx, float cy);

	ENTITY_TYPES myType;
	float x, y;
};

/// <summary>
/// 특정 개체의 캐릭터 이동
/// </summary>
struct SCPacketMoveCharacter : public Packet
{
	SCPacketMoveCharacter(PID cid, float nx, float ny);

	float x, y;
};

/// <summary>
/// 특정 개체의 시야 탈출
/// </summary>
struct SCPacketDisppearCharacter : public Packet
{
	SCPacketDisppearCharacter(PID cid);
};

/// <summary>
/// 대화 메시지, 대상이 -1이면 모두에게 보낸다.
/// </summary>
struct SCPacketChatMessage : public Packet
{
	SCPacketChatMessage(PID tid, const WCHAR* msg, size_t length);
	SCPacketChatMessage(PID tid, const WCHAR msg[]);

	WCHAR Caption[100];
};

/// <summary>
/// 특정 플레이어의 캐릭터 삭제 (나간 플레이어 이외에 다른 플레이어에 전송)
/// </summary>
struct SCPacketSignOut : public Packet
{
	SCPacketSignOut(PID pid, UINT users);

	UINT usersCurrent;
};
#pragma pack(pop)

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
