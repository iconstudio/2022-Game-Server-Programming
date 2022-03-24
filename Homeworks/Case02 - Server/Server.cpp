#include "stdafx.h"
#include "ServerFramework.h"
#include "Session.h"

ServerFramework framework{};

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

	framework.ProceedWorld(session, send_bytes);
}

void ErrorDisplay(const char* title)
{
	WCHAR lpMsgBuf[501];
	ZeroMemory(lpMsgBuf, 501);

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		lpMsgBuf, 500, NULL);

	char msg[501];
	ZeroMemory(msg, 501);

	wcstombs_s(NULL, msg, 500, lpMsgBuf, 500);

	cout << title << " -> 오류: " << msg << "\n";

	LocalFree(lpMsgBuf);
}
