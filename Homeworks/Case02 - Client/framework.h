#pragma once
#include "Player.h"

#define WM_SOCKET (WM_USER + 1)

enum class States : int
{
	Begin = 0,
	Game = 1
};

class Framework
{
public:
	Framework();

	void Init(HWND window);
	void Start();
	void WINAPI Communicate(UINT msg, WPARAM sock, LPARAM state);
	void Render(HWND window);
	void InputEvent(WPARAM key);
	void EnterIpChar(WPARAM key);
	void SendKey(WPARAM key);

	COLORREF background_color = C_WHITE;

private:
	States Status = States::Begin;

	INT ID = -1; // 네트워크 상에서 플레이어의 번호
	PlayerCharacter m_Player;
	WPARAM Lastkey;
	std::string Server_IP;

	SOCKET Socket;
	SOCKADDR_IN Server_address;
	INT sz_Address;
	WSABUF Buffer_recv;
	WSABUF Buffer_send;
	WSABUF Buffer_world[2];
	CHAR CBuffer_world[BUFFSIZE];
	INT Buffer_world_length;
	vector<PlayerCharacter> World_instances;

	HWND Window;
	HDC DC_double;
	HBITMAP Surface_double;

	HDC Board_canvas;
	HBITMAP Board_image;
	RECT Board_rect;
};
