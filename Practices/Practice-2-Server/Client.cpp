#include "pch.hpp"
#include "Network.hpp"
#include "Client.hpp"

Client::Client(SOCKET sk)
	: socket(sk), sz_recv(0), sz_send(0) {
	ZeroMemory(&overlap, sizeof(overlap));
	wbuffer.len = sz_buffer;
	wbuffer.buf = buffer;
}

void Client::SetSocket(SOCKET sk) {
	socket = sk;
}

void Client::SetOverlap(WSAOVERLAPPED&& ov) {
	overlap = std::move(ov);
}

void Client::SetEvent(WSAEVENT&& ev) {
	overlap.hEvent = std::move(ev);
}

WSAOVERLAPPED& Client::GetOverlap() {
	return overlap;
}
