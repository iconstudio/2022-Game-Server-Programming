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
/// ������ �α��� �ǻ� �˸�
/// </summary>
struct CSPacketSignIn : public Packet
{
	CSPacketSignIn(const CHAR* nickname);

	CHAR Nickname[30];
};

/// <summary>
/// �������� ������ �ǻ� �˸�
/// </summary>
struct CSPacketSignOut : public Packet
{
	CSPacketSignOut(PID pid);
};

/// <summary>
/// ������ �Է��� Ű �˸�
/// </summary>
struct CSPacketKeyInput : public Packet
{
	CSPacketKeyInput(PID pid, WPARAM key);

	WPARAM Key;
};

/// <summary>
/// ���� ������ �÷��̾�� ID �ο�, ���� ������ ��, �ִ� ������ �� �˸���
/// </summary>
struct SCPacketSignUp : public Packet
{
	SCPacketSignUp(PID nid, UINT users = 0, UINT usersmax = 0);

	UINT usersCurrent, usersMax;
};

/// <summary>
/// Ư�� �÷��̾��� ĳ���� ����
/// </summary>
struct SCPacketCreateCharacter : public Packet
{
	SCPacketCreateCharacter(PID pid, CHAR cx, CHAR cy);

	CHAR x, y;
};

/// <summary>
/// Ư�� �÷��̾��� ĳ���� �̵�
/// </summary>
struct SCPacketMoveCharacter : public Packet
{
	SCPacketMoveCharacter(PID pid, CHAR nx, CHAR ny);

	CHAR x, y;
};

/// <summary>
/// Ư�� �÷��̾��� ĳ���� ���� (���� �÷��̾� �̿ܿ� �ٸ� �÷��̾ ����)
/// </summary>

struct SCPacketSignOut : public Packet
{
	SCPacketSignOut(PID pid, UINT users);

	UINT usersCurrent;
};
#pragma pack(pop)
