#pragma once
#define WM_SOCKET (WM_USER + 1)
#include "Network.hpp"
#include "Player.h"

enum class GAME_STATES : UCHAR
{
	Begin = 0,
	Game = 1
};

class Session
{
public:
	PlayerCharacter* Instance;
};

void CALLBACK CallbackRecv(DWORD error, DWORD bytes, LPWSAOVERLAPPED overlap, DWORD flags);
void CALLBACK CallbackSend(DWORD error, DWORD bytes, LPWSAOVERLAPPED overlap, DWORD flags);

class ClientFramework
{
public:
	ClientFramework();

	void Init(HWND window);
	void Start();
	void Update();
	void Communicate(UINT msg, WPARAM sock, LPARAM state);
	void Render(HWND window);

	friend void CALLBACK CallbackRecv(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
	friend void CALLBACK CallbackSend(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);

	void InputEvent(WPARAM key);
	void EnterIpChar(WPARAM key);
	void SendKey(WPARAM key);

	UINT ID = -1; // 네트워크 상에서 플레이어의 번호

	COLORREF background_color = C_WHITE;

private:
	GAME_STATES Status = GAME_STATES::Begin;

	SOCKET Socket;
	SOCKADDR_IN serverAddress;
	INT serverAddressSize;
	std::string serverIP;

	vector<Session*> Clients;
	PlayerCharacter* myCharacter;
	WPARAM Lastkey;

	WSAOVERLAPPED recvOverlap;
	WSABUF recvBuffer;
	CHAR recvCBuffer[BUFFSIZE];
	DWORD recvBytes;

	HWND Window;

	HDC doubleDCSurface;
	HBITMAP doubleDCBitmap;

	HDC boardSurface;
	HBITMAP boardBitmap;
	RECT boardArea;
};
