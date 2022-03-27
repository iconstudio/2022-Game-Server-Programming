#include "stdafx.h"
#include "ServerFramework.h"
#include "Session.h"

ServerFramework framework{};
WCHAR Msg_buffer[501];
char Msg[501];

int main()
{
	framework.Init();
	framework.Start();

	cout << "서버 종료됨\n";
	while (true);

	return 0;
}

void CallbackStartPositions(DWORD err, DWORD recv_bytes, LPWSAOVERLAPPED over, DWORD flags)
{
	auto session = framework.GetClient(over);
	if (!session)
	{
		ErrorDisplay("No client session.");
		return;
	}

	if (0 != err || 0 == recv_bytes)
	{
		framework.RemoveSession(session->ID);
		ErrorDisplay("CallbackStartPositions()");
		return;
	}

	session->ProceedStartPosition(recv_bytes);
}

void CallbackInputs(DWORD err, DWORD recv_bytes, LPWSAOVERLAPPED over, DWORD flags)
{
	auto session = framework.GetClient(over);
	if (!session)
	{
		ErrorDisplay("No client session.");
		return;
	}

	if (0 != err || 0 == recv_bytes)
	{
		framework.RemoveSession(session->ID);
		ErrorDisplay("CallbackInputs()");
		return;
	}

	session->ProceedKeyInput(recv_bytes);
}

void CallbackWorld(DWORD err, DWORD send_bytes
	, LPWSAOVERLAPPED over, DWORD flags)
{
	auto session = framework.GetClient(over);
	if (!session)
	{
		ErrorDisplay("No client session.");
		return;
	}

	if (0 != err || 0 == send_bytes)
	{
		framework.RemoveSession(session->ID);
		ErrorDisplay("CallbackWorld()");
		return;
	}

	session->ProceedWorld(send_bytes);
}

void ErrorDisplay(const char* title)
{
	ZeroMemory(Msg_buffer, 501);

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		Msg_buffer, 500, NULL);

	ZeroMemory(Msg, 501);

	wcstombs_s(NULL, Msg, 500, Msg_buffer, 500);

	cout << title << " -> 오류: " << Msg << "\n";
}
