#pragma once
#include "targetver.h"

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

template<typename Type>
using shared_atomic = atomic<shared_ptr<Type>>;
template<typename Type>
using weak_atomic = atomic<weak_ptr<Type>>;

template<typename Type>
using shared_atomic_concurrent_vector = concurrent_vector<shared_atomic<Type>>;
template<typename Type>
using weak_atomic_concurrent_vector = concurrent_vector<weak_atomic<Type>>;

constexpr double PI = 3.141592653589793;
constexpr COLORREF C_BLACK = RGB(0, 0, 0);
constexpr COLORREF C_WHITE = RGB(255, 255, 255);
constexpr COLORREF C_GREEN = RGB(0, 128, 0);
constexpr COLORREF C_GOLD = RGB(223, 130, 20);
constexpr int CLIENT_W = 800;
constexpr int CLIENT_H = 600;

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

class Panel;
class Framework;
class Scene;
enum class SCENE_STATES : UCHAR;

class Asynchron;
class Packet;
class Session;
enum class ASYNC_OPERATIONS : UCHAR;
enum class PACKET_TYPES : UCHAR;
enum class SESSION_STATES;

class GameTransform;
class GameCamera;
class GameObject;

// 고유한 식별자가 있는 게임 객체
class GameEntity;

using PID = long long;
class PlayerCharacter;
class NonPlayerable;
enum class ENTITY_TYPES;

void ErrorAbort(const wchar_t* title, const int errorcode);
void ErrorDisplay(const wchar_t* title, const int errorcode);
void ErrorAbort(const wchar_t* title);
void ErrorDisplay(const wchar_t* title);

constexpr size_t BUFFSZ = 512;
constexpr PID PLAYERS_ID_BEGIN = 10000;
constexpr ULONG CLIENTS_MAX_NUMBER = 5000;
