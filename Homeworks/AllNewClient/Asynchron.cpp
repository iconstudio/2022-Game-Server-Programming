#include "pch.hpp"
#include "Asynchron.hpp"

void ClearOverlap(WSAOVERLAPPED* overlap)
{
	ZeroMemory(overlap, sizeof(WSAOVERLAPPED));
}

void ClearOverlap(Asynchron* overlap)
{
	if (overlap->sendBuffer)
	{
		overlap->sendBuffer.reset();
	}

	if (overlap->sendCBuffer)
	{
		overlap->sendCBuffer.reset();
	}

	ZeroMemory(overlap, sizeof(Asynchron));
}
