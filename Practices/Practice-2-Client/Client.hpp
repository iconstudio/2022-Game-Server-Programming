#pragma once

class Client {
public:
	Client(SOCKET sk);

	WSAOVERLAPPED overlap;
	SOCKET socket;
	u_int sz_recv, sz_send;
};
