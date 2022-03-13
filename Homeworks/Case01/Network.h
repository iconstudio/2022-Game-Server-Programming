#pragma once

enum PACKET_TYPES : int
{
	NONE = 0,
	CONNECTED,

	CREATE_CELL,
	CREATE_PLAYER,
	PLAYER_MOVE = 10
};

struct BasicPacket
{
	PACKET_TYPES Type = NONE;
};

struct Client
{
	SOCKET m_Socket;
	int Index = 0;
};
