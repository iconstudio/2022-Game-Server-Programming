#include "pch.hpp"
#include "Session.hpp"
#include "GameObject.hpp"

Session::Session()
	: myStatus(ST_FREE), myID(PID(-1)), mySocket(NULL)
	, myAvatar(nullptr)
	, myNickname()
	, recvBytes(0)
{
	recvOverlap.myOperation = OP_RECV;

	next_move_time = std::chrono::system_clock::now() + std::chrono::seconds(1);
}

Session::~Session()
{
	myAvatar.store(nullptr);
}

void Session::Start()
{
	auto avatar = AcquireAvatar();
	avatar->x = rand() % W_WIDTH;
	avatar->y = rand() % W_HEIGHT;
	ReleseAvatar(avatar);
}

void Session::Release()
{
	ZeroMemory(myNickname, sizeof(myNickname));
	recvBytes = 0;

	closesocket(mySocket);
}

void Session::SetStatus(SESSION_STATE status)
{
	myStatus.store(status, std::memory_order_relaxed);
}

void Session::SetID(PID id)
{
	myID.store(id, std::memory_order_relaxed);
}

void Session::SetSocket(SOCKET sock)
{
	mySocket.store(sock, std::memory_order_relaxed);
}

void Session::SetAvatar(shared_ptr<GameObject> handle)
{
	myAvatar.store(handle, std::memory_order_relaxed);
}

SESSION_STATE Session::AcquireStatus()
{
	return myStatus.load(std::memory_order_acquire);
}

void Session::ReleseStatus(SESSION_STATE status)
{
	myStatus.store(status, std::memory_order_release);
}

SESSION_STATE Session::GetStatus() const
{
	return myStatus.load(std::memory_order_relaxed);
}

shared_ptr<GameObject> Session::AcquireAvatar()
{
	return myAvatar.load(std::memory_order_acquire);
}

void Session::ReleseAvatar(shared_ptr<GameObject> handle)
{
	myAvatar.store(handle, std::memory_order_release);
}

const shared_ptr<GameObject> Session::GetAvatar() const
{
	return myAvatar.load(std::memory_order_relaxed);
}

shared_ptr<GameObject> Session::GetAvatar()
{
	return myAvatar.load(std::memory_order_relaxed);
}

void Session::do_send(const void* packet)
{
	auto special = new Asynchron(reinterpret_cast<const char*>(packet));
	WSASend(mySocket, &special->_wsabuf, 1, 0, 0, &special->_over, 0);
}

void Session::send_login_info_packet(size_t number_users)
{
	auto handle = AcquireAvatar();
	SC_LOGIN_OK_PACKET p{ myID, number_users, MAX_USER };
	p.x = handle->x;
	p.y = handle->y;
	ReleseAvatar(handle);

	do_send(&p);
}

void Session::send_move_packet(PID c_id, shared_ptr<Session> other, int client_time)
{
	if (c_id <= 0 || PID(-1) == c_id)
	{
		std::cout << "ROOR2";
		return;
	}

	auto avatar = other->AcquireAvatar();
	SC_MOVE_OBJECT_PACKET p{ c_id, avatar->x, avatar->y };
	p.client_time = client_time;
	other->ReleseAvatar(avatar);

	do_send(&p);
}

void Session::send_add_object(PID c_id, shared_ptr<Session> other)
{
	if (c_id <= 0 || PID(-1) == c_id)
	{
		std::cout << "ROOR";
		return;
	}

	auto other_avatar = other->AcquireAvatar();
	SC_ADD_OBJECT_PACKET p{ c_id, PLAYER_CATEGORY::HUMAN, p.myNickname, other_avatar->x, other_avatar->y };
	other->ReleseAvatar(other_avatar);

	do_send(&p);
}

void Session::send_remove_object(PID c_id)
{
	SC_REMOVE_OBJECT_PACKET p{ c_id };

	do_send(&p);
}
