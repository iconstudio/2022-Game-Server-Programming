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
class Asynchron;
class Packet;
enum class ASYNC_OPERATIONS;
enum class PACKET_TYPES;

class Session;
enum class SESSION_STATES;
class LocalSession;
class SightSector;
class SightInstances;
class Timer;

class GameTransform;
class GameEntity;
class GameCamera;
class GameObject;
class GameStaticObject;

class PlayerCharacter;
class NonPlayerable;

void ErrorDisplay(const char* title);
