#pragma once
#include "Asynchron.hpp"

enum SESSION_STATE { ST_FREE, ST_ACCEPTED, ST_INGAME };

class Session
{
public:
	Session();
	~Session();

	void Start();
	void Release();

	void SetStatus(SESSION_STATE status);
	void SetID(PID id);
	void SetSocket(SOCKET sock);
	void SetAvatar(shared_ptr<GameObject> handle);

	SESSION_STATE AcquireStatus();
	void ReleseStatus(SESSION_STATE status);
	SESSION_STATE GetStatus() const;

	shared_ptr<GameObject> AcquireAvatar();
	void ReleseAvatar(shared_ptr<GameObject> handle);
	const shared_ptr<GameObject> GetAvatar() const;
	shared_ptr<GameObject> GetAvatar();

	void do_recv()
	{
		DWORD recv_flag = 0;

		memset(&recvOverlap._over, 0, sizeof(recvOverlap._over));

		recvOverlap._wsabuf.len = BUF_SIZE - recvBytes;
		recvOverlap._wsabuf.buf = recvOverlap._send_buf + recvBytes;

		WSARecv(mySocket, &recvOverlap._wsabuf, 1, 0, &recv_flag,
			&recvOverlap._over, 0);
	}

	void do_send(const void* packet, size_t size);

	void send_login_info_packet(size_t number_users);

	void send_move_packet(PID c_id, shared_ptr<Session> other, int client_time);

	void send_add_object(PID c_id, shared_ptr<Session> other);

	void send_remove_object(PID c_id);

	Asynchron recvOverlap;

public:
	atomic<SESSION_STATE> myStatus;
	atomic<PID> myID;
	atomic<SOCKET> mySocket;

	char myNickname[NAME_SIZE];
	shared_atomic<GameObject> myAvatar;

	std::unordered_set<PID> view_list;
	std::mutex cvSight;

	std::chrono::system_clock::time_point next_move_time;
	int recvBytes;
};
