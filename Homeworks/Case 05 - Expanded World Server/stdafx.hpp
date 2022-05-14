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
#include <atomic>
#include <concurrent_vector.h>

using std::array;
using std::shared_ptr;
using std::unique_ptr;
using std::weak_ptr;
using std::atomic;
using std::atomic_bool;
using std::atomic_int;
using std::atomic_uint;

class IOCPFramework;
class SightSector;
class SightInstances;
class SightController;
class Timer;

class Asynchron;
class Packet;
enum class ASYNC_OPERATIONS : UCHAR;
enum class PACKET_TYPES : UCHAR;

class Session;
class LocalSession;
enum class SESSION_STATES;

class GameTransform;
class GameEntity;
class GameCamera;
class GameObject;
class GameStaticObject;

class PlayerCharacter;
class NonPlayerable;

void ClearOverlap(WSAOVERLAPPED* overlap);
void ClearOverlap(Asynchron* overlap);
void ErrorDisplay(const char* title);
