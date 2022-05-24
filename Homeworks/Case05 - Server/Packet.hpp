#pragma once
#include "stdafx.hpp"

#pragma pack(push, 1)
class Packet
{
public:
	Packet(PACKET_TYPES type, USHORT size, PID pid);
	Packet(PACKET_TYPES type, PID pid = 0);
	virtual ~Packet();

	USHORT Size;
	PACKET_TYPES Type;
	PID playerID;
};

enum class PACKET_TYPES : UCHAR
{
	NONE = 0,
	CS_SIGNIN,
	CS_SIGNOUT,
	CS_KEY,
	CS_CHAT,

	SC_SIGNUP,
	SC_CREATE_PLAYER,
	SC_SIGNOUT,
	SC_CHAT,

	SC_APPEAR_CHARACTER,
	SC_DISAPPEAR_CHARACTER,
	SC_MOVE_CHARACTER,
};
#pragma pack(pop)
