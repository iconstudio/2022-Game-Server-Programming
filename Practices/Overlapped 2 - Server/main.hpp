#pragma once
#pragma comment(lib, "ws2_32")
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <Windows.h>

#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <span>
#include <chrono>
#include <atomic>
#include <thread>
#include <mutex>
#include <future>
#include <exception>
#include <memory>
#include <fstream>

constexpr int SERVER_PORT = 9000;
constexpr int SZ_BUFFER = 512;

struct Client
{
	Client();

	WSAOVERLAPPED Overlap;
	SOCKET Socket;
	WSABUF Buffer;
	char Back_buffer[SZ_BUFFER];
	int sz_recv, sz_sent;
};

DWORD WINAPI ServerIOWorker(LPVOID param);
void CALLBACK CompletionRoutine(DWORD error, DWORD traffic, LPOVERLAPPED overlapped, DWORD flags);
