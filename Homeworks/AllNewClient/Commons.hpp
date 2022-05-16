#pragma once
#include "Scene.hpp"
#include "Packet.hpp"

class SceneMain : public Scene
{
public:
	SceneMain(Framework& framework);

	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update(float time_elapsed) override;
	virtual void Render(HDC surface) override;
	virtual void Reset() override;
	virtual void Complete() override;
};

class SceneLoading : public Scene
{
public:
	SceneLoading(Framework& framework);

	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update(float time_elapsed) override;
	virtual void Render(HDC surface) override;
	virtual void Reset() override;
	virtual void Complete() override;

};

class SceneGame : public Scene
{
public:
	SceneGame(Framework& framework);

	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update(float time_elapsed) override;
	virtual void Render(HDC surface) override;
	virtual void Reset() override;
	virtual void Complete() override;
};

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
/// 특정 플레이어의 캐릭터 생성
/// </summary>
struct SCPacketCreateCharacter : public Packet
{
	SCPacketCreateCharacter(PID pid, CHAR cx, CHAR cy);

	CHAR x, y;
};

/// <summary>
/// 특정 플레이어의 캐릭터 이동
/// </summary>
struct SCPacketMoveCharacter : public Packet
{
	SCPacketMoveCharacter(PID pid, CHAR nx, CHAR ny);

	CHAR x, y;
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
