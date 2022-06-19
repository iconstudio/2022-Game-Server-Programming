#include "pch.hpp"
#include "stdafx.hpp"
#include "Packet.hpp"
#include "Commons.hpp"

Packet::Packet(PACKET_TYPES type, UCHAR size, PID pid)
	: Type(type), Size(size), playerID(pid)
{}

Packet::Packet(PACKET_TYPES type, PID pid)
	: Packet(type, sizeof(Packet), pid)
{}

Packet::~Packet()
{}
