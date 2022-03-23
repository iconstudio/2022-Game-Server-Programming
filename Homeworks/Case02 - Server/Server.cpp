#include "stdafx.h"
#include "ServerFramework.h"
#include "Session.h"

ServerFramework Framework{};

int main()
{
	Framework.Init();
	Framework.Start();

	cout << "서버 종료됨\n";
	while (true);

	return 0;
}

void CallbackStartPositions(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags)
{
	auto session = Framework.GetClient(over);
	auto& wbuffer = session->Buffer_recv;
	auto& cbuffer = wbuffer.buf;
	auto& sz_recv = wbuffer.len;

	const size_t sz_want = sizeof(Position);
	if (sz_want <= num_bytes)
	{
		auto player_pos = reinterpret_cast<Position*>(cbuffer);
		Player player{ player_pos->x, player_pos->y };
		cout << "플레이어 좌표: (" << player_pos->x << ", " << player_pos->y << ")\n";

		session->ClearRecvBuffer();
	}
	else
	{

	}
}

void CallbackInputs(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags)
{

}

void ErrorDisplay(const char* title)
{
	TCHAR* lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&lpMsgBuf, 0, NULL);

	wcout << title << " -> 오류: " << lpMsgBuf << endl;

	LocalFree(lpMsgBuf);
}
