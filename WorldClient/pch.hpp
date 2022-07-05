#pragma once
#include "stdafx.hpp"
#include "../WorldServer/Commons.hpp"

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

class GameCamera;
class GameObject;
class GameSprite;

// 고유한 식별자가 있는 게임 객체
class GameEntity;

class PlayerCharacter;
class NonPlayerable;

constexpr COLORREF C_BLACK = RGB(0, 0, 0);
constexpr COLORREF C_WHITE = RGB(255, 255, 255);
constexpr COLORREF C_GREEN = RGB(0, 128, 0);
constexpr COLORREF C_GOLD = RGB(223, 130, 20);

#endif
