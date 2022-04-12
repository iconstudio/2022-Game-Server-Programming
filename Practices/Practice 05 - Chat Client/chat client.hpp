#pragma once
#pragma comment (lib, "WS2_32.LIB")

#include <iostream>
#include <WS2tcpip.h>
using namespace std;

constexpr auto BUFSIZE = 512;
const char* SERVER_ADDR = "127.0.0.1";
constexpr short SERVER_PORT = 4000;
