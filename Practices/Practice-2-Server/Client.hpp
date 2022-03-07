#pragma once
#include "pch.hpp"

class Client {
public:
	Client(SOCKET sk);

	SOCKET socket;
	DWORD sz_recv, sz_send;
	const DWORD sz_buffer = SZ_BUFF;
	WSABUF wbuffer;
	CHAR buffer[SZ_BUFF + 1];

	void SetSocket(SOCKET sk);
	void SetOverlap(WSAOVERLAPPED&& ov);
	void SetEvent(WSAEVENT&& ev);
	WSAOVERLAPPED& GetOverlap();

private:
	WSAOVERLAPPED overlap;
};
