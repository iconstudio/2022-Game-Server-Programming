#include "pch.hpp"
#include "SceneMain.hpp"
#include "Framework.hpp"
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
	constexpr auto notification = L"접속할 서버의 IP 주소를 입력해주세요.";
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
	if (PACKET_TYPES::CS_SIGNIN == packet.Type)
	{
		// 로그인 요청 전송 성공
		Complete();
		return true;
	}
	else if (PACKET_TYPES::CS_MOVE == packet.Type)
	{
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
