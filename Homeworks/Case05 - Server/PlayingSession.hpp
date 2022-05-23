#pragma once
#include "Session.h"

class PlayingSession : public Session
{
public:
	PlayingSession(UINT index, PID id, SOCKET sock, IOCPFramework& framework);
	virtual ~PlayingSession();

	bool IsConnected() const volatile;
	bool IsDisconnected() const volatile;
	bool IsAccepted() const volatile;
	bool IsPlayer() const volatile;
	bool IsNonPlayer() const volatile;
};
