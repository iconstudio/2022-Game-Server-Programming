#pragma once
#include "pch.hpp"

constexpr COLORREF C_BLACK = RGB(0, 0, 0);
constexpr COLORREF C_WHITE = RGB(255, 255, 255);
constexpr COLORREF C_GREEN = RGB(0, 128, 0);
constexpr COLORREF C_GOLD = RGB(223, 130, 20);
constexpr int CLIENT_W = 800;
constexpr int CLIENT_H = 600;

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

// ������ �ĺ��ڰ� �ִ� ���� ��ü
class GameEntity;

using PID = long long;
class PlayerCharacter;
class NonPlayerable;
enum class ENTITY_TYPES;

constexpr size_t BUFFSZ = 512;
constexpr PID PLAYERS_ID_BEGIN = 10000;
constexpr ULONG CLIENTS_MAX_NUMBER = 5000;
