#pragma once
#pragma comment(lib, "MSWSock.lib")
#pragma comment(lib, "Ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WS2tcpip.h>
#include <MSWSock.h>

#include <iostream>
#include <memory>
#include <numeric>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <initializer_list>
#include <array>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <concurrent_vector.h>

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

class IOCPFramework;
class Session;
enum class SESSION_STATES;

class GameObject
{
public:
	INT x, y;
};

/// <summary>
/// 플레이어 캐릭터
/// </summary>
class PlayerCharacter : public GameObject
{
public:
	PlayerCharacter(INT ox, INT oy);

	bool TryMoveLT();
	bool TryMoveRT();
	bool TryMoveUP();
	bool TryMoveDW();
};

void ErrorDisplay(const char* title);

constexpr SIZE_T WND_SZ_W = 800, WND_SZ_H = 600; // 창 크기
constexpr SIZE_T CELL_SIZE = 64;

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

constexpr int BOARD_CELLS_CNT_H = 16;
constexpr int BOARD_CELLS_CNT_V = 16;
constexpr int BOARD_W = CELL_W * BOARD_CELLS_CNT_H;
constexpr int BOARD_H = CELL_H * BOARD_CELLS_CNT_V;
constexpr int BOARD_X = (WND_SZ_W - BOARD_W) / 2;
constexpr int BOARD_Y = (WND_SZ_H - BOARD_H - 20) / 2;
