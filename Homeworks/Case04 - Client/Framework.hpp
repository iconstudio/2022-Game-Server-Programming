#pragma once
#include "stdafx.hpp"
#include "Network.hpp"
#include "NetworkBackend.hpp"
#include "Sprite.hpp"
#include "PlayerCharacter.hpp"

enum class GAME_STATES : UCHAR
{
	Begin = 0,
	Connect,
	Game
};

void CALLBACK CallbackRecv(DWORD error, DWORD bytes, LPWSAOVERLAPPED overlap, DWORD flags);
void CALLBACK CallbackSend(DWORD error, DWORD bytes, LPWSAOVERLAPPED overlap, DWORD flags);

class ClientFramework
{
public:
	ClientFramework();
	~ClientFramework();

	void Init(HWND window);
	void Connect();
	void Start();

	void InputEvent(WPARAM key);
	void InputIpChar(WPARAM key);
	void InputKey(WPARAM key);
	void Render(HWND window);

	void ProceedRecv(DWORD bytes);
	void ProceedSend(EXOVERLAPPED* overlap, DWORD bytes);

	void AddClient(const PID id);
	LocalSession* GetClient(const PID id);
	void RemoveClient(const PID id);

	friend void CALLBACK CallbackRecv(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
	friend void CALLBACK CallbackSend(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);

	std::vector<std::shared_ptr<Sprite>> Sprites;

	GAME_STATES Status;
	UINT ID; // 플레이어의 번호
	std::string Nickname;
	PlayerCharacter* myCharacter;

	COLORREF background_color = C_WHITE;

private:
	void SendSignInMsg();
	void SendSignOutMsg();
	void SendSignKeyMsg();

	int Recv(DWORD begin_bytes = 0);
	int Send(LPWSABUF datas, UINT count, LPWSAOVERLAPPED overlap);
	int SendPacket(Packet* packet, ULONG size);

	SOCKET Socket;
	SOCKADDR_IN serverAddress;
	INT serverAddressSize;
	std::string serverIP;

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

	std::vector<LocalSession*> Clients;
	std::unordered_map<PID, LocalSession*> ClientsDict;
	UINT clientNumber, clientMaxNumber;
	WPARAM Lastkey;
};
