#include "pch.hpp"
#include "Framework.hpp"
#include "Scene.hpp"
#include "Packet.hpp"

Framework::Framework()
	: myScenes(), myState(nullptr)
	, isPaused(false)
	, appSurface(NULL), appPainter()
{
	myScenes.reserve(10);
}

Framework::~Framework()
{}

void Framework::Push(Scene* scene)
{
	Push(shared_ptr<Scene>(scene));
}

void Framework::Awake()
{
	if (0 < myScenes.size())
	{
		for (auto& scene : myScenes)
		{
			scene->Awake();
		}
	}
}

void Framework::Start()
{
	if (0 < myScenes.size())
	{
		std::reverse(myScenes.begin(), myScenes.end());
		myState = Pop();

		myState->Start();
	}
}

void Framework::Update(float elapsed_time)
{
	if (myState)
	{
		myState->Update(elapsed_time);

		if (myState->IsCompleted())
		{
			TryPop();
		}
	}
}

void Framework::BeginRendering(HWND hwnd)
{
	appSurface = BeginPaint(hwnd, &appPainter);

	Render(appSurface);

	ReleaseDC(hwnd, appSurface);
	EndPaint(hwnd, &appPainter);
}

void Framework::Render(HDC surface)
{
	if (myState)
	{
		myState->Render(surface);
	}
}

void Framework::OnNetwork(const Packet& packet)
{
	if (myState)
	{
		myState->OnNetwork(packet);
	}
}

void Framework::OnMouse(WPARAM button, LPARAM cursor)
{
	if (myState)
	{
		myState->OnMouse(button, cursor);
	}
}

void Framework::OnKeyboard(WPARAM key, LPARAM states)
{
	if (myState)
	{
		myState->OnKeyboard(key, states);
	}
}

void Framework::OnWindow(WPARAM aevent, LPARAM params)
{
	if (myState)
	{
		myState->OnWindow(aevent, params);
	}
}

bool Framework::IsPaused() const
{
	return isPaused;
}

bool Framework::TryPause()
{
	if (!IsPaused())
	{
		Pause();

		return true;
	}

	return false;
}

void Framework::Pause()
{
	isPaused = true;
}

void Framework::Resume()
{
	isPaused = false;
}

void Framework::Push(const shared_ptr<Scene>& scene)
{
	myScenes.push_back(scene);
}

void Framework::Push(shared_ptr<Scene>&& scene)
{
	myScenes.push_back(std::forward<shared_ptr<Scene>>(scene));
}

bool Framework::TryPop()
{
	if (0 < myScenes.size())
	{
		myState = Pop();

		myState->Start();

		return true;
	}

	return false;
}

shared_ptr<Scene>& Framework::Pop()
{
	auto& result = myScenes.back();
	myScenes.pop_back();

	return result;
}
