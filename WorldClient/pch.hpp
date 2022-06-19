#pragma once
#include "stdafx.hpp"
#include "Commons.hpp"

#ifndef __PCH__
#define __PCH__

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

class PlayerCharacter;
class NonPlayerable;
enum class ENTITY_TYPES : UCHAR;

constexpr size_t BUFFSZ = 512;
constexpr PID PLAYERS_ID_BEGIN = 10000;
constexpr ULONG CLIENTS_MAX_NUMBER = 5000;

#endif
