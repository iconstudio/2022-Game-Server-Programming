#pragma once
#ifndef __PCH__
#define __PCH__
#include "stdafx.hpp"

class Framework;
class Asynchron;
struct Packet;
enum OPERATION_TYPES : unsigned char;
enum PACKET_TYPES : unsigned char;
enum DIRECTIONS : unsigned char;
enum CHAT_TYPES : unsigned char;

enum CHARACTER_TYPES : unsigned char;
enum CLASS_TYPES : unsigned char;

class ClientSlot;
class Session;
enum CLIENT_STATES : unsigned char;

class GameObject;
class GameEntity; // place
class PlayerCharacter;
class NPC;
class Monster;

class Sprite;

#include "ServerProtocol.hpp"

#endif
