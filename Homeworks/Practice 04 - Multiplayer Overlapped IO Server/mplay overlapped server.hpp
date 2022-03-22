#pragma once

#pragma warning(disable: 4244)
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "msimg32.lib")
#pragma comment(lib, "Ws2_32.lib")
#include <windows.h>
#include <WS2tcpip.h>

#include <iostream>
#include <vector>
#include <unordered_map>
#include <thread>
#include <algorithm>

constexpr int BUF_SIZE = 256;

void ErrorAbort(const wchar_t* title);
void ErrorDisplay(const wchar_t* title);
void CALLBACK RecvRoutine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void CALLBACK SendRoutine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);

class Session
{
public:
	INT id;
	SOCKET Socket;
	WSAOVERLAPPED Overlap;
	WSABUF Buffer_recv, Buffer_send;
	char cbuffer_recv[BUF_SIZE], cbuffer_send[BUF_SIZE];

	Session();
	Session(Session&) = default;
	Session(INT nid, SOCKET sock);

	void ClearOverlap();
	void ProceedReceive();

	friend void CALLBACK RecvRoutine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
	friend void CALLBACK SendRoutine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
};
