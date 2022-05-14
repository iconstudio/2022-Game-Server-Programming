#pragma once
#pragma comment(lib, "MSWSock.lib")
#pragma comment(lib, "Ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WS2tcpip.h>
#include <MSWSock.h>

#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
using DirectX::BoundingBox, DirectX::BoundingOrientedBox, DirectX::BoundingFrustum;
using DirectX::XMFLOAT3, DirectX::XMFLOAT4, DirectX::XMFLOAT4X4;
using DirectX::XMStoreFloat3, DirectX::XMStoreFloat4, DirectX::XMStoreFloat4x4;
using DirectX::XMLoadFloat3, DirectX::XMLoadFloat4, DirectX::XMLoadFloat4x4;

#include <string>
#include <iostream>
#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>
#include <memory>
#include <numeric>
#include <concurrent_vector.h>
#include <vector>
#include <array>
#include <unordered_map>
#include <algorithm>

using std::array;
using std::shared_ptr;
using std::unique_ptr;
using std::weak_ptr;
using std::atomic;
using std::atomic_bool;
using std::atomic_int;
using std::atomic_uint;

using int_pair = std::pair<int, int>;
using float_pair = std::pair<float, float>;

template<typename Type>
using shared_concurrent_vector = Concurrency::concurrent_vector<shared_ptr<Type>>;
template<typename Type>
using concurrent_vector = Concurrency::concurrent_vector<Type>;

class IOCPFramework;
class Timer;
class SightController;
class SightSector;

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
