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

using PID = long long;

class IOCPFramework;
class Session;
enum class SESSION_STATES;

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

constexpr SIZE_T WND_SZ_W = 800, WND_SZ_H = 600; // 창 크기
constexpr SIZE_T CELL_SIZE = 64;
constexpr int CELL_W = CELL_SIZE;
constexpr int CELL_H = CELL_SIZE;
constexpr int CELLS_CNT_H = 8;
constexpr int CELLS_CNT_V = 8;
constexpr int CELLS_LENGTH = CELLS_CNT_H * CELLS_CNT_V;
constexpr int BOARD_W = CELL_W * CELLS_CNT_H;
constexpr int BOARD_H = CELL_H * CELLS_CNT_V;
constexpr int BOARD_X = (WND_SZ_W - BOARD_W) / 2;
constexpr int BOARD_Y = (WND_SZ_H - BOARD_H - 20) / 2;
