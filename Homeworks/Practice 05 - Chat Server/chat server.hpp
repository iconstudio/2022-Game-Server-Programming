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
#include <iostream>
#include <unordered_map>
#include <WS2tcpip.h>
#pragma comment(lib, "WS2_32.lib")

using namespace std;
constexpr int PORT_NUM = 4000;
constexpr int BUF_SIZE = 256;

class SESSION;

void ErrorAbort(const wchar_t* title);
void ErrorDisplay(const wchar_t* title);
void CALLBACK RecvRoutine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void CALLBACK SendRoutine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
