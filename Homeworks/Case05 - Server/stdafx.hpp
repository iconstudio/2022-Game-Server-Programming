#pragma once
#include "pch.hpp"

class IOCPFramework;
class Session;
class Packet;
class Asynchron;
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

constexpr UINT PLAYERS_MAX_NUMBER = 10000;
constexpr UINT NPC_MAX_NUMBER = 10000;
constexpr UINT ENTITIES_MAX_NUMBER = PLAYERS_MAX_NUMBER + NPC_MAX_NUMBER;
constexpr UINT CLIENTS_ORDER_BEGIN = NPC_MAX_NUMBER;

constexpr PID NPC_ID_BEGIN = 0;
constexpr PID CLIENTS_ID_BEGIN = PID(NPC_MAX_NUMBER);

constexpr float CELL_SIZE = 32.0f;
constexpr float CELL_W = CELL_SIZE;
constexpr float CELL_H = CELL_SIZE;

constexpr int WORLD_CELLS_CNT_H = 400;
constexpr int WORLD_CELLS_CNT_V = 400;
constexpr float WORLD_W = CELL_W * WORLD_CELLS_CNT_H;
constexpr float WORLD_H = CELL_H * WORLD_CELLS_CNT_V;

constexpr int SIGHT_CELLS_CNT_H = 11;
constexpr int SIGHT_CELLS_CNT_V = 11;
constexpr int SIGHT_CELLS_RAD_H = SIGHT_CELLS_CNT_H / 2;
constexpr int SIGHT_CELLS_RAD_V = SIGHT_CELLS_CNT_H / 2;

constexpr float SIGHT_W = CELL_W * SIGHT_CELLS_CNT_H;
constexpr float SIGHT_H = CELL_H * SIGHT_CELLS_CNT_V;
constexpr float SIGHT_RAD_W = CELL_W * SIGHT_CELLS_RAD_H;
constexpr float SIGHT_RAD_H = CELL_H * SIGHT_CELLS_RAD_V;
