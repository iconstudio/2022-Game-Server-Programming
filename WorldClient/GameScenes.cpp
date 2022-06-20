#include "pch.hpp"
#include "GameScenes.hpp"
#include "Framework.hpp"
#include "GameCamera.hpp"
#include "PlayerCharacter.hpp"
#include "Draw.hpp"

SceneGame::SceneGame(Framework& framework)
	: Scene(framework, "SceneGame", 10)
	, myLocalInstances()
	, myPlayerCharacter(nullptr)
	, myWorldImage(NULL), myWorldImageContext(NULL)
	, myRandomEngine(), myTerrainRandomizer(0, 255)
	, myWorldRegions(), myWorldTerrain()
{
	myLocalInstances.reserve(100);
	myWorldRegions.reserve(20);

	for (int y = 0; y < WORLD_CELLS_CNT_V; y++)
	{
		auto& row = myWorldTerrain[y];

		for (int x = 0; x < WORLD_CELLS_CNT_H; x++)
		{
			auto& column = row[x];

			column = myTerrainRandomizer(myRandomEngine);
		}
	}
}

void SceneGame::Awake()
{
	myWorldImageContext = GetDC(NULL);

	//myWorldImage = CreateCompatipleBitmap(myWorldImageContext);
}

void SceneGame::Start()
{}

void SceneGame::Update(float time_elapsed)
{
}

void SceneGame::Reset()
{
	myCamera.myPosition[0] = 0.0f;
	myCamera.myPosition[1] = 0.0f;
}

void SceneGame::Complete()
{
	myCamera.myPosition[0] = 0.0f;
	myCamera.myPosition[1] = 0.0f;

	myLocalInstances.clear();

	Scene::Complete();
}

void SceneGame::Render(HDC surface)
{
	const auto cam_pos = myCamera.myPosition;

	auto old_bk = SetBkMode(surface, TRANSPARENT);
	Draw::Clear(surface, CLIENT_W, CLIENT_H, C_GREEN);

	constexpr COLORREF outer_color = 0;
	auto m_hPen = CreatePen(PS_NULL, 1, outer_color);
	auto m_oldhPen = HPEN(Draw::Attach(surface, m_hPen));

	auto m_hBR = CreateSolidBrush(outer_color);
	auto m_oldhBR = HBRUSH(Draw::Attach(surface, m_hBR));

	Draw::SizedRect(surface, 40-cam_pos[0], 80-cam_pos[1], 300, 300);
	//Draw::SizedRect(surface, 0, 0, width, height);

	Draw::Detach(surface, m_oldhBR, m_hBR);
	Draw::Detach(surface, m_oldhPen, m_hPen);

	Scene::Render(surface);
	SetBkMode(surface, old_bk);
}

bool SceneGame::OnNetwork(const Packet& packet)
{
	const auto& pid = packet.playerID;
	const auto& packet_type = packet.Type;
	const auto& packet_sz = packet.Size;
	const auto& handle = myFramework.GetHandle();

	// 접속 전까진 -1
	const auto my_id = myFramework.GetMyID();

	if (PACKET_TYPES::SC_SIGNUP == packet_type)
	{
		return true;
	}
	else if (PACKET_TYPES::CS_MOVE == packet_type)
	{
		return true;
	}
	else if (PACKET_TYPES::SC_SIGNUP == packet_type)
	{
		return true;
	}
	else if (PACKET_TYPES::SC_CREATE_PLAYER == packet_type)
	{
		auto ticket = const_cast<Packet*>(&packet);
		const auto rp = static_cast<SCPacketCreatePlayer*>(ticket);

		return true;
	}
	else if (PACKET_TYPES::SC_APPEAR_OBJ == packet_type)
	{
		auto ticket = const_cast<Packet*>(&packet);
		const auto rp = static_cast<SCPacketAppearEntity*>(ticket);

		auto it = myLocalInstances.find(pid);

		if (myLocalInstances.end() != it)
		{
			auto& instance = *it->second;
			// 속성 갱신
			instance.myID = pid;
			instance.myCategory = rp->myCategory;
			instance.myType = rp->myType;
			instance.myMaxHP = rp->maxhp;
			instance.myHP = rp->hp;
			instance.myMaxMP = rp->maxmp;
			instance.myMP = rp->mp;
			instance.myArmour = rp->amour;

			// 위치 갱신
			instance.myPosition[0] = rp->x;
			instance.myPosition[1] = rp->y;

			instance.Show();
		}
		else if (IsPlayer(pid)) // 플레이어의 캐릭터
		{
			auto instance = CreateInstance<PlayerCharacter>();
			instance->myID = pid;
			instance->myCategory = rp->myCategory;
			instance->myType = rp->myType;
			instance->myMaxHP = rp->maxhp;
			instance->myHP = rp->hp;
			instance->myMaxMP = rp->maxmp;
			instance->myMP = rp->mp;
			instance->myArmour = rp->amour;
			instance->myPosition[0] = rp->x;
			instance->myPosition[1] = rp->y;

			if (!myPlayerCharacter && PID(-1) != my_id && pid == my_id)
			{
				myPlayerCharacter = instance;
			}

			myLocalInstances.try_emplace(pid, instance);
		}
		else // NPC
		{
			auto instance = CreateInstance<PlayerCharacter>();
			instance->myID = pid;
			instance->myCategory = rp->myCategory;
			instance->myType = rp->myType;
			instance->myMaxHP = rp->maxhp;
			instance->myHP = rp->hp;
			instance->myMaxMP = rp->maxmp;
			instance->myMP = rp->mp;
			instance->myArmour = rp->amour;
			instance->myPosition[0] = rp->x;
			instance->myPosition[1] = rp->y;

			myLocalInstances.try_emplace(pid, instance);
		}

		if (myPlayerCharacter)
		{
			UpdateCamera(myPlayerCharacter);
		}

		InvalidateRect(handle, NULL, TRUE);

		return true;
	}
	else if (PACKET_TYPES::SC_DISAPPEAR_OBJ == packet_type)
	{
		// 다른 플레이어의 캐릭터 삭제
		auto rit = myLocalInstances.find(pid);

		if (myLocalInstances.end() != rit)
		{
			rit->second->Hide();
			//DestroyInstance(rit->second);

			myLocalInstances.erase(rit);

			InvalidateRect(handle, NULL, TRUE);
		}

		return true;
	}
	else if (PACKET_TYPES::SC_MOVE_OBJ == packet_type)
	{
		auto ticket = const_cast<Packet*>(&packet);
		const auto rp = static_cast<SCPacketMoveCharacter*>(ticket);

		auto mit = myLocalInstances.find(pid);

		if (myLocalInstances.end() != mit)
		{
			auto& instance = mit->second;
			instance->myPosition[0] = rp->x;
			instance->myPosition[1] = rp->y;

			InvalidateRect(handle, NULL, TRUE);
		}

		return true;
	}
	else if (PACKET_TYPES::SC_STAT_OBJ == packet.Type)
	{
		return true;
	}
	else if (PACKET_TYPES::SC_SIGNIN_FAILED == packet_type)
	{
		return true;
	}

	return false;
}

void SceneGame::OnKeyDown(WPARAM key, LPARAM states)
{
	switch (key)
	{
		case VK_LEFT:
		{
			myFramework.myNetwork.SendMoveDir(MOVE_TYPES::LEFT);
		}
		break;

		case VK_RIGHT:
		{
			myFramework.myNetwork.SendMoveDir(MOVE_TYPES::RIGHT);
		}
		break;

		case VK_UP:
		{
			myFramework.myNetwork.SendMoveDir(MOVE_TYPES::UP);
		}
		break;

		case VK_DOWN:
		{
			myFramework.myNetwork.SendMoveDir(MOVE_TYPES::DOWN);
		}
		break;
	}
}

void SceneGame::OnKeyUp(WPARAM key, LPARAM states)
{
	switch (key)
	{
		case VK_LEFT:
		{
			myFramework.myNetwork.SendAttack(MOVE_TYPES::LEFT);
		}
		break;

		case VK_RIGHT:
		{
			myFramework.myNetwork.SendAttack(MOVE_TYPES::RIGHT);
		}
		break;

		case VK_UP:
		{
			myFramework.myNetwork.SendAttack(MOVE_TYPES::UP);
		}
		break;

		case VK_DOWN:
		{
			myFramework.myNetwork.SendAttack(MOVE_TYPES::DOWN);
		}
		break;
	}
}
