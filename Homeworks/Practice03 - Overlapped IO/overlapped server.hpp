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
#include <memory>
#include <thread>
#include <algorithm>
