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
struct CSPacketKeyInput : public Packet
{
	CSPacketKeyInput(PID pid, WPARAM key);

	WPARAM Key;
};

/// <summary>
/// 새로 접속한 플레이어에게 ID 부여, 현재 동접자 수, 최대 동접자 수 알리기
/// </summary>
struct SCPacketSignUp : public Packet
{
	SCPacketSignUp(PID nid, UINT users = 0, UINT usersmax = 0);

	UINT usersCurrent, usersMax;
};

/// <summary>
/// 특정 플레이어의 로컬 세션 생성 (첫 접속 시에만 실행)
/// </summary>
struct SCPacketCreatePlayer : public Packet
{
	SCPacketCreatePlayer(PID pid, int cx, int cy);

	int x, y;
};

/// <summary>
/// 특정 개체의 시야 진입
/// </summary>
struct SCPacketAppearCharacter : public Packet
{
	SCPacketAppearCharacter(PID cid, int type, int cx, int cy);

	int myType;
	int x, y;
};

/// <summary>
/// 특정 개체의 시야 탈출
/// </summary>
struct SCPacketDisppearCharacter : public Packet
{
	SCPacketDisppearCharacter(PID cid);
};

/// <summary>
/// 특정 개체의 캐릭터 이동
/// </summary>
struct SCPacketMoveCharacter : public Packet
{
	SCPacketMoveCharacter(PID cid, int nx, int ny);

	int x, y;
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
