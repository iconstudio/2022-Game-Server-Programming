#include "stdafx.h"
#include "ServerFramework.h"
#include "Session.h"

ServerFramework framework{};
TCHAR* Msg_buffer = NULL;

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
		ErrorDisplay("CallbackStartPositions: No client session.");
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
		ErrorDisplay("CallbackInputs: No client session.");
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
		ErrorDisplay("CallbackWorld: No client session.");
		return;
	}

	if (0 != err || 0 == send_bytes)
	{
		if (WSA_IO_PENDING != err)
		{
			//framework.RemoveSession(session->ID);
			//ErrorDisplay("CallbackWorld()");
			//return;
		}
	}
	else
	{
		session->ProceedWorld(send_bytes);
	}
}

void ErrorDisplay(const char* title)
{
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(TCHAR*)&Msg_buffer, 0, NULL);

	cout << title << " -> 오류: ";
	setlocale(LC_ALL, "KOREAN");
	wprintf(L"%s\n", Msg_buffer);

	LocalFree(Msg_buffer);
}
