#include "pch.hpp"
#include "SceneLoading.hpp"
#include "Framework.hpp"
#include "Draw.hpp"

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

	constexpr auto notification = L"접속 중 입니다...";
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
		return false; // 처리는 안 하지만 종료는 함.
	}
	else if (PACKET_TYPES::CS_MOVE == packet.Type)
	{
		return true;
	}

	return false;
}
