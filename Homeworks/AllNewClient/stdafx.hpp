#pragma once
#include "pch.hpp"

constexpr COLORREF C_BLACK = RGB(0, 0, 0);
constexpr COLORREF C_WHITE = RGB(255, 255, 255);
constexpr COLORREF C_GREEN = RGB(0, 128, 0);
constexpr COLORREF C_GOLD = RGB(223, 130, 20);
constexpr int CLIENT_W = 800;
constexpr int CLIENT_H = 600;
constexpr float FRAME_W = 800.0f;
constexpr float FRAME_H = 600.0f;

constexpr int CELL_SIZE = 32;

constexpr int CELL_W = CELL_SIZE;
constexpr int CELL_H = CELL_SIZE;
constexpr int CELLS_CNT_H = 2000;
constexpr int CELLS_CNT_V = 2000;
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
constexpr int BOARD_X = (FRAME_W - BOARD_W) / 2;
constexpr int BOARD_Y = (FRAME_H - BOARD_H - 20) / 2;

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
enum class ENTITY_TYPES : UCHAR;

constexpr size_t BUFFSZ = 512;
constexpr PID PLAYERS_ID_BEGIN = 10000;
constexpr ULONG CLIENTS_MAX_NUMBER = 5000;
