#pragma once

enum COMP_TYPE { OP_ACCEPT, OP_RECV, OP_SEND };

class Asynchron
{
public:
	WSAOVERLAPPED _over;

	WSABUF _wsabuf;
	char _send_buf[BUF_SIZE];
	COMP_TYPE myOperation;

	// 수신
	Asynchron()
	{
		_wsabuf.len = BUF_SIZE;
		_wsabuf.buf = _send_buf;
		myOperation = OP_RECV;

		ZeroMemory(_send_buf, sizeof(_send_buf));
		Clear();
	}

	// 송신
	Asynchron(const char* packet, size_t size)
	{
		_wsabuf.len = size;
		_wsabuf.buf = _send_buf;

		ZeroMemory(_send_buf, sizeof(_send_buf));
		Clear();

		myOperation = OP_SEND;
		memcpy(_send_buf, packet, size);
	}

	void Clear()
	{
		ZeroMemory(&_over, sizeof(_over));
	}
};
