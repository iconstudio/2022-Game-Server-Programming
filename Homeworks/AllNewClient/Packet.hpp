#pragma once

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
#pragma pack(pop)

enum class PACKET_TYPES : UCHAR
{
	NONE = 0,
	CS_SIGNIN,
	CS_SIGNOUT,
	CS_KEY,
	SC_SIGNUP,
	SC_CREATE_CHARACTER,
	SC_APPEAR_CHAR,
	SC_DISAPPEAR_CHAR,
	SC_MOVE_CHARACTER,
	SC_SIGNOUT
};
