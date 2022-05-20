#include "pch.hpp"
#include "stdafx.hpp"
#include "GameScenes.hpp"
#include "Framework.hpp"
#include "Commons.hpp"
#include "Packet.hpp"
#include "GameCamera.hpp"
#include "PlayerCharacter.hpp"
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
	auto old_bk = SetBkMode(surface, TRANSPARENT);

	auto old_align = SetTextAlign(surface, TA_CENTER);
	constexpr auto notification = L"������ ������ IP �ּҸ� �Է����ּ���.";
	auto noti_len = lstrlen(notification);

	size_t ip_size = 16;
	WCHAR ip_address[17];
	mbstowcs_s(&ip_size, ip_address, streamIP.c_str(), 16);

	TextOut(surface, draw_x, draw_y - 70, notification, noti_len);
	TextOut(surface, draw_x, draw_y, ip_address, lstrlen(ip_address));

	SetTextAlign(surface, old_align);

	SetBkMode(surface, old_bk);
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

bool SceneMain::OnNetwork(const Packet& packet)
{
	if (PACKET_TYPES::CS_SIGNIN == packet.Type) // �α��� ��û ���� ����
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
	Draw::Clear(surface, CLIENT_W, CLIENT_H, C_WHITE);

	auto filler = CreateSolidBrush(C_BLACK);
	auto old_filler = Draw::Attach(surface, filler);
	auto old_bk = SetBkMode(surface, TRANSPARENT);

	auto old_align = SetTextAlign(surface, TA_CENTER);

	constexpr auto notification = L"���� �� �Դϴ�...";
	auto noti_len = lstrlen(notification);
	TextOut(surface, draw_x, draw_y, notification, noti_len);

	SetTextAlign(surface, old_align);
	SetBkMode(surface, old_bk);
	Draw::Detach(surface, old_filler, filler);
}

void SceneLoading::Reset()
{}

void SceneLoading::Complete()
{
	Scene::Complete();
}

bool SceneLoading::OnNetwork(const Packet& packet)
{
	if (PACKET_TYPES::SC_SIGNUP == packet.Type)
	{
		Complete();
		return false; // ó���� �� ������ ����� ��.
	}

	return false;
}

SceneGame::SceneGame(Framework& framework)
	: Scene(framework, "SceneGame", 10)
	, myLocalInstances()
	, myPlayerCharacter(nullptr)
{
	myLocalInstances.reserve(100);
}

void SceneGame::Awake()
{}

void SceneGame::Start()
{}

void SceneGame::Update(float time_elapsed)
{}

void SceneGame::Reset()
{
	myCamera.myPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);
}

void SceneGame::Complete()
{
	myCamera.myPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);
	myLocalInstances.clear();

	Scene::Complete();
}

void SceneGame::Render(HDC surface)
{
	Draw::Clear(surface, CLIENT_W, CLIENT_H, C_GREEN);

	auto old_bk = SetBkMode(surface, TRANSPARENT);
	Scene::Render(surface);
	SetBkMode(surface, old_bk);
}

bool SceneGame::OnNetwork(const Packet& packet)
{
	const auto& pid = packet.playerID;
	const auto& packet_type = packet.Type;
	const auto& packet_sz = packet.Size;

	if (PACKET_TYPES::SC_SIGNUP == packet_type)
	{
		return true;
	}
	else if (PACKET_TYPES::SC_CREATE_PLAYER == packet_type)
	{
		auto ticket = const_cast<Packet*>(&packet);
		const auto rp = static_cast<SCPacketCreatePlayer*>(ticket);

		if (PID(-1) != pid && pid == myFramework.GetMyID())
		{
			// �� ĳ���ʹ� �̶� �����Ѵ�.
			myPlayerCharacter = CreateInstance<PlayerCharacter>();
			myPlayerCharacter->myPosition.x = 30.0f;
			myPlayerCharacter->myPosition.y = 30.0f;
			myPlayerCharacter->myID = pid;

			myLocalInstances.push_back(myPlayerCharacter);
			InvalidateRect(NULL, NULL, TRUE);
		}
		else
		{
			// �ٸ� �÷��̾��� ĳ���ʹ� �̶� �����Ѵ�.
		}	

		return true;
	}
	else if (PACKET_TYPES::SC_APPEAR_CHARACTER == packet_type)
	{
		auto ticket = const_cast<Packet*>(&packet);
		const auto rp = static_cast<SCPacketAppearCharacter*>(ticket);

		for (const auto& inst : myLocalInstances)
		{
			if (inst->myID == pid)
			{
				inst->myPosition.x = rp->x;
				inst->myPosition.y = rp->y;

				InvalidateRect(NULL, NULL, TRUE);
				return true;
			}
		}

		// �ٸ� �÷��̾��� ĳ����
		auto inst = CreateInstance<PlayerCharacter>();
		inst->myPosition.x = rp->x;
		inst->myPosition.y = rp->y;
		inst->myID = pid;

		myLocalInstances.push_back(inst);
		InvalidateRect(NULL, NULL, TRUE);

		return true;
	}
	else if (PACKET_TYPES::SC_DISAPPEAR_CHARACTER == packet_type)
	{
		// �ٸ� �÷��̾��� ĳ���� ����
		auto rit = std::remove_if(myLocalInstances.begin(), myLocalInstances.end(),
			[pid](const GameEntity* entity) -> bool {
			return (entity->myID == pid);
		});
				
		if (myLocalInstances.end() != rit)
		{
			myLocalInstances.erase(rit);

			InvalidateRect(NULL, NULL, TRUE);
		}

		return true;
	}
	else if (PACKET_TYPES::SC_MOVE_CHARACTER == packet_type)
	{
		auto ticket = const_cast<Packet*>(&packet);
		const auto rp = static_cast<SCPacketMoveCharacter*>(ticket);

		return true;
	}
	else if (PACKET_TYPES::SC_SIGNOUT == packet_type)
	{
		return true;
	}

	return false;
}

void SceneGame::OnKeyDown(WPARAM key, LPARAM states)
{

}

template<>
GameEntity* Scene::CreateInstance<GameEntity, GameEntity, true>()
{
	auto ptr = new GameEntity();
	AddInstance(ptr);
	return ptr;
}

template<>
PlayerCharacter* Scene::CreateInstance<PlayerCharacter, PlayerCharacter, true>()
{
	auto ptr = new PlayerCharacter();
	AddInstance(ptr);
	return ptr;
}
