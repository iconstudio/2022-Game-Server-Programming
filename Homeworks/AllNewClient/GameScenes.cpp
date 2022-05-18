#include "pch.hpp"
#include "stdafx.hpp"
#include "GameScenes.hpp"
#include "Framework.hpp"
#include "PlayerCharacter.hpp"
#include "Commons.hpp"
#include "Packet.hpp"
#include "Draw.hpp"

SceneMain::SceneMain(Framework& framework)
	: Scene(framework, "SceneMain", 0)
	, streamIP("127.0.0.1")
	, streamRect{ draw_x - 50, draw_y - 15, draw_x + 50, draw_y - 15 }
{}

void SceneMain::Awake()
{}

void SceneMain::Start()
{}

void SceneMain::Update(float time_elapsed)
{}

void SceneMain::Render(HDC surface)
{
	Draw::Clear(surface, CLIENT_W, CLIENT_H, C_WHITE);

	auto filler = CreateSolidBrush(C_BLACK);
	auto old_filler = Draw::Attach(surface, filler);

	auto old_align = SetTextAlign(surface, TA_CENTER);
	constexpr auto notification = L"접속할 서버의 IP 주소를 입력해주세요.";
	auto noti_len = lstrlen(notification);

	size_t ip_size = 16;
	WCHAR ip_address[17];
	mbstowcs_s(&ip_size, ip_address, streamIP.c_str(), 16);

	TextOut(surface, draw_x, draw_y - 70, notification, noti_len);
	TextOut(surface, draw_x, draw_y, ip_address, lstrlen(ip_address));

	SetTextAlign(surface, old_align);

	Draw::Detach(surface, old_filler, filler);
}

void SceneMain::Reset()
{
	streamIP = "";
}

void SceneMain::Complete()
{
	Scene::Complete();
}

bool SceneMain::OnNetwork(Packet* packet)
{
	if (PACKET_TYPES::CS_SIGNIN == packet->Type) // 로그인 요청 전송 성공
	{
		Complete();
		return true;
	}

	return false;
}

void SceneMain::OnKeyDown(WPARAM key, LPARAM states)
{
	switch (key)
	{
		case VK_RETURN:
		{
			if (7 < streamIP.length())
			{
				myFramework.Connect(streamIP.c_str());

				InvalidateRect(NULL, &streamRect, FALSE);
			}
		}
		break;

		case VK_ESCAPE:
		{
			streamIP.clear();
			InvalidateRect(NULL, &streamRect, FALSE);
		}
		break;

		case VK_BACK:
		{
			if (0 < streamIP.length())
			{
				streamIP.erase(streamIP.end() - 1);
			}
			InvalidateRect(NULL, &streamRect, FALSE);
		}
		break;

		case VK_OEM_PERIOD:
		{
			if (streamIP.length() < 15)
			{
				streamIP.push_back('.');
				InvalidateRect(NULL, &streamRect, FALSE);
			}
		}
		break;

		default:
		{
			if (isdigit(int(key)))
			{
				if (streamIP.length() < 16)
				{
					streamIP.push_back(char(key));
					InvalidateRect(NULL, &streamRect, FALSE);
				}
			}
		}
		break;
	}
}

SceneLoading::SceneLoading(Framework& framework)
	: Scene(framework, "SceneLoading", 0)
	, streamRect{ draw_x - 50, draw_y - 15, draw_x + 50, draw_y - 15 }
	, myLife(0.0f)
{}

void SceneLoading::Awake()
{}

void SceneLoading::Start()
{
	myLife = 20.0f;
}

void SceneLoading::Update(float time_elapsed)
{
	if (myLife -= time_elapsed; myLife <= 0)
	{
		myFramework.JumpToPrevScene();
	}
}

void SceneLoading::Render(HDC surface)
{
	auto filler = CreateSolidBrush(C_BLACK);
	auto old_filler = Draw::Attach(surface, filler);

	auto old_align = SetTextAlign(surface, TA_CENTER);

	constexpr auto notification = L"접속 중 입니다...";
	auto noti_len = lstrlen(notification);
	TextOut(surface, draw_x, draw_y, notification, noti_len);

	SetTextAlign(surface, old_align);
	Draw::Detach(surface, old_filler, filler);
}

void SceneLoading::Reset()
{}

void SceneLoading::Complete()
{
	Scene::Complete();
}

bool SceneLoading::OnNetwork(Packet* packet)
{
	if (PACKET_TYPES::SC_SIGNUP == packet->Type)
	{
		Complete();
		return true;
	}

	return false;
}

SceneGame::SceneGame(Framework& framework)
	: Scene(framework, "SceneGame", 10)
	, myLocalInstances()
{}

void SceneGame::Awake()
{}

void SceneGame::Start()
{}

void SceneGame::Update(float time_elapsed)
{}

void SceneGame::Render(HDC surface)
{}

void SceneGame::Reset()
{}

void SceneGame::Complete()
{}

bool SceneGame::OnNetwork(Packet* packet)
{
	const auto& pid = packet->playerID;
	const auto& packet_type = packet->Type;
	const auto& packet_sz = packet->Size;

	if (PACKET_TYPES::SC_SIGNUP == packet_type)
	{
		auto inst = new PlayerCharacter();
		inst->myID = pid;

		myLocalInstances.push_back(inst);
	
		return true;
	}
	else if (PACKET_TYPES::SC_CREATE_PLAYER == packet_type)
	{
		return true;
	}
	else if (PACKET_TYPES::SC_APPEAR_CHARACTER == packet_type)
	{
		const auto rp = static_cast<SCPacketAppearCharacter*>(packet);

		for (const auto& inst : myLocalInstances)
		{
			if (inst->myID == pid)
			{
				return false;
			}
		}

		// 다른 플레이어의 캐릭터
		auto inst = CreateInstance<PlayerCharacter>(rp->x, rp->y);
		inst->myID = pid;

		myLocalInstances.push_back(inst);

		return true;
	}
	else if (PACKET_TYPES::SC_DISAPPEAR_CHARACTER == packet_type)
	{
		// 다른 플레이어의 캐릭터 삭제
		auto rit = std::erase_if(myLocalInstances,
			[pid](const GameEntity* entity) -> bool {
			return (entity->myID == pid);
		});

		for (auto it = myLocalInstances.begin(); it != myLocalInstances.end(); it++)
		{
			auto& inst = *it;

			if (inst->myID == pid)
			{
				myLocalInstances.erase(it);
				break;
			}
		}
	
		return true;
	}
	else if (PACKET_TYPES::SC_MOVE_CHARACTER == packet_type)
	{
		return true;
	}

	return false;
}

void SceneGame::OnKeyDown(WPARAM key, LPARAM states)
{

}

