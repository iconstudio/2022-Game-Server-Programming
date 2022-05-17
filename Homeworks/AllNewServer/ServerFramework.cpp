#include "pch.hpp"
#include "stdafx.hpp"
#include "ServerFramework.hpp"
#include "Asynchron.hpp"

ServerFramework::ServerFramework()
{
	setlocale(LC_ALL, "KOREAN");
	std::cout.sync_with_stdio(false);

	ClearOverlap(&acceptOverlap);
	ZeroMemory(acceptCBuffer, sizeof(acceptCBuffer));

	for (int i = 0; i < CLIENTS_MAX_NUMBER; ++i)
	{
		auto& empty = clientsPool.at(i);
		empty = std::make_shared<Session>(i, -1, NULL, *this);
	}
}

ServerFramework::~ServerFramework()
{

	WSACleanup();
}

void ServerFramework::Awake()
{

}

void ServerFramework::Start()
{

}