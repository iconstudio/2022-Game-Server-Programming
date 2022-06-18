#pragma once

#pragma pack(push, 1)
class Packet
{
public:
	Packet(PACKET_TYPES type, UCHAR size, PID pid);
	Packet(PACKET_TYPES type, PID pid = 0);
	virtual ~Packet();

	UCHAR Size;
	PACKET_TYPES Type;
	PID playerID;
};
#pragma pack(pop)
