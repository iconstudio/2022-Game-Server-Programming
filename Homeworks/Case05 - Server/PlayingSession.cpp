#include "pch.hpp"
#include "PlayingSession.hpp"

PlayingSession::PlayingSession(UINT index, PID id, SOCKET sock, IOCPFramework& framework)
	: Session(index, id, sock, framework)
{

}

PlayingSession::~PlayingSession()
{

}

bool PlayingSession::IsConnected() const volatile
{
	return SESSION_STATES::CONNECTED == Status.load(std::memory_order_relaxed);
}

bool PlayingSession::IsDisconnected() const volatile
{
	return SESSION_STATES::NONE == Status.load(std::memory_order_relaxed);
}

bool PlayingSession::IsAccepted() const volatile
{
	return SESSION_STATES::ACCEPTED == Status.load(std::memory_order_relaxed);
}

bool PlayingSession::IsPlayer() const volatile
{
	return true;
}

bool PlayingSession::IsNonPlayer() const volatile
{
	return false;
}
