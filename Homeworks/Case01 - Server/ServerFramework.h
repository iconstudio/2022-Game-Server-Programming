#pragma once

class ServerFramework
{
public:
	ServerFramework();

	void Start();

	SOCKET Socket;
	SOCKADDR_IN Address;
	INT sz_Address;
	WSAOVERLAPPED Overlap_recv;
	LPWSAOVERLAPPED Overlap_send;
};

