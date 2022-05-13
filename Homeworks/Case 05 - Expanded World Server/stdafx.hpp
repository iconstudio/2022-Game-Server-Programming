#pragma once
#pragma comment(lib, "MSWSock.lib")
#pragma comment(lib, "Ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WS2tcpip.h>
#include <MSWSock.h>

#include <iostream>
#include <memory>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <numeric>
#include <mutex>
#include <chrono>
#include <array>
#include <concurrent_vector.h>

class IOCPFramework;

class Asynchron;
class Packet;

class Session;
enum class SESSION_STATES;
class LocalSession;
class SightSector;
class SightInstances;

class GameTimer;
class GameTransform;
class GameObject;
class GameStaticObject;

class PlayerCharacter;
class NonPlayerable;

/// <summary>
/// 플레이어 캐릭터
/// </summary>
class PlayerCharacter
{
public:
	PlayerCharacter(CHAR ox, CHAR oy);

	bool TryMoveLT();
	bool TryMoveRT();
	bool TryMoveUP();
	bool TryMoveDW();

	CHAR x, y;
};

void ErrorDisplay(const char* title);
