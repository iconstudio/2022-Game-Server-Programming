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
#include <optional>
#include <memory>
#include <numeric>
#include <concurrent_vector.h>
#include <concurrent_unordered_map.h>
#include <concurrent_unordered_set.h>
#include <vector>
#include <array>
#include <unordered_map>
#include <algorithm>

using std::string;
using std::array;
using std::shared_ptr;
using std::unique_ptr;
using std::weak_ptr;
using std::atomic;
using std::atomic_bool;
using std::atomic_int;
using std::atomic_uint;
using std::make_shared;
using std::make_unique;

using int_pair = std::pair<int, int>;
using float_pair = std::pair<float, float>;

template<typename Type>
using concurrent_vector = Concurrency::concurrent_vector<Type>;
template<typename Type>
using shared_concurrent_vector = concurrent_vector<shared_ptr<Type>>;
template<typename Type>
using weak_concurrent_vector = concurrent_vector<weak_ptr<Type>>;
template<typename Type>
using unique_concurrent_vector = concurrent_vector<unique_ptr<Type>>;

template<typename KeyType, typename ValueType>
using concurrent_map = Concurrency::concurrent_unordered_map<KeyType, ValueType>;
template<typename KeyType>
using concurrent_set = Concurrency::concurrent_unordered_set<KeyType>;
template<typename KeyType, typename ValueType>
using shared_concurrent_map = concurrent_map<KeyType, shared_ptr<ValueType>>;
template<typename KeyType>
using shared_concurrent_set = Concurrency::concurrent_unordered_set<shared_ptr<KeyType>>;

template<typename Type>
using shared_atomic = atomic<shared_ptr<Type>>;
template<typename Type>
using weak_atomic = atomic<weak_ptr<Type>>;

template<typename Type>
using shared_atomic_concurrent_vector = concurrent_vector<shared_atomic<Type>>;
template<typename Type>
using weak_atomic_concurrent_vector = concurrent_vector<weak_atomic<Type>>;
template<typename KeyType, typename ValueType>
using shared_atomic_concurrent_map = concurrent_map<KeyType, shared_atomic<ValueType>>;
template<typename KeyType>
using shared_atomic_concurrent_set = Concurrency::concurrent_unordered_set<shared_atomic<KeyType>>;

constexpr double PI = 3.141592653589793;

template<typename T>
constexpr T dcos(T value)
{
	return std::cos(value * PI / 180);
}

template<typename T>
constexpr T dsin(T value)
{
	return std::sin(value * PI / 180);
}

using PID = long long;
class IOCPFramework;
class Session;
class Packet;
class Asynchron;
class EXOVERLAPPED;
enum class SESSION_STATES;
enum class OVERLAP_OPS : UCHAR;
enum class PACKET_TYPES : UCHAR;

class GameCamera;
class SightManager;
class SightSector;

class GameObject;
class GameEntity;
class PlayerCharacter;

void ErrorDisplay(const char* title);

constexpr USHORT PORT = 6000;
constexpr PID NPC_ORDER_BEGIN = 0;
constexpr UINT CLIENTS_MAX_NUMBER = 10000;
constexpr PID CLIENTS_ORDER_BEGIN = 10000;
constexpr UINT ENTITIES_MAX_NUMBER = CLIENTS_ORDER_BEGIN + 10000;
constexpr UINT BUFFSIZE = 512;

constexpr int CELL_SIZE = 32;

constexpr int CELL_W = CELL_SIZE;
constexpr int CELL_H = CELL_SIZE;
constexpr int CELLS_CNT_H = 400;
constexpr int CELLS_CNT_V = 400;
constexpr int CELLS_LENGTH = CELLS_CNT_H * CELLS_CNT_V;
constexpr int WORLD_W = CELL_W * CELLS_CNT_H;
constexpr int WORLD_H = CELL_H * CELLS_CNT_V;

constexpr int SIGHT_CELLS_CNT_H = 11;
constexpr int SIGHT_CELLS_CNT_V = 11;
constexpr int SIGHT_W = CELL_W * SIGHT_CELLS_CNT_H;
constexpr int SIGHT_H = CELL_H * SIGHT_CELLS_CNT_V;
