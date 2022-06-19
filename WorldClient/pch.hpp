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

// ������ �ĺ��ڰ� �ִ� ���� ��ü
class GameEntity;

class PlayerCharacter;
class NonPlayerable;

#endif
