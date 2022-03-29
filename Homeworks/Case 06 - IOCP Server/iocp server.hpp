#pragma once
#pragma comment(lib, "WS2_32.lib")
#pragma warning(disable: 4244)
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <WS2tcpip.h>

#include <iostream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <concurrent_unordered_map.h>
#include <array>

using namespace std;

constexpr int PORT_NUM = 4000;
constexpr int BUF_SIZE = 256;
constexpr int MAX_USER = 10;

