#pragma once
#include "Scene.hpp"
#include "Packet.hpp"

class SceneMain : public Scene
{
public:
	SceneMain(Framework& framework);

	void Awake() override;
	void Start() override;
	void Update(float time_elapsed) override;
	void Render(HDC surface) override;
	void Reset() override;
	void Complete() override;

	void OnNetwork(const Packet& packet) override;
	void OnKeyDown(WPARAM key, LPARAM states) override;

private:
	string streamIP;

	const int draw_x = CLIENT_W / 2;
	const int draw_y = CLIENT_H / 2;
	const RECT streamRect;
};

class SceneLoading : public Scene
{
public:
	SceneLoading(Framework& framework);

	void Awake() override;
	void Start() override;
	void Update(float time_elapsed) override;
	void Render(HDC surface) override;
	void Reset() override;
	void Complete() override;

	void OnNetwork(const Packet& packet) override;

private:
	const int draw_x = CLIENT_W / 2;
	const int draw_y = CLIENT_H / 2 - 70;
	const RECT streamRect;

	float myLife;
};

class SceneGame : public Scene
{
public:
	SceneGame(Framework& framework);

	void Awake() override;
	void Start() override;
	void Update(float time_elapsed) override;
	void Render(HDC surface) override;
	void Reset() override;
	void Complete() override;

	void OnNetwork(const Packet& packet) override;
	void OnKeyDown(WPARAM key, LPARAM states) override;

private:

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
/// Ư�� �÷��̾��� ĳ���� ���� (ù ���� �ÿ��� ����)
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
