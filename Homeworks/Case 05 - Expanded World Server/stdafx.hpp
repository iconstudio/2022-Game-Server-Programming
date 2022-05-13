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
class GameEntity;
class GameObject;
class GameStaticObject;

class PlayerCharacter;
class NonPlayerable;

void ErrorDisplay(const char* title);
