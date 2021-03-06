#include "pch.hpp"
#include "Framework.hpp"
#include "Network.hpp"
#include "Scene.hpp"

Framework::Framework(Network& network)
	: myScenes(), myStages(), myState(nullptr)
	, appHandle(NULL), myNetwork(network), myTasks()
	, myViewFollower(nullptr)
	, isPaused(false)
	, appSurface(NULL), appPainter()
{
	myTasks.reserve(1000);
	myScenes.reserve(10);
	myStages.reserve(10);
}

Framework::~Framework()
{}

void Framework::SetHandle(HWND window)
{
	appHandle = window;
}

HWND Framework::GetHandle() const
{
	return appHandle;
}

void Framework::AddRoom(Scene* scene)
{
	Register(Push(scene));
}

bool Framework::JumpToNextScene()
{
	if (0 < myStages.size() && myStages.rend() != myStageIterator)
	{
		myState = Pop();
		myState->Start();
		InvalidateRect(NULL, NULL, FALSE);

		return true;
	}

	return false;
}

bool Framework::JumpToPrevScene()
{
	if (0 < myStages.size() && myStages.rbegin() != myStageIterator)
	{
		myState = *(--myStageIterator);
		myState->Start();
		InvalidateRect(NULL, NULL, FALSE);

		return true;
	}

	return false;
}

bool Framework::JumpTo(const char* scene_name)
{
	auto scene = GetScene(scene_name);
	auto it = std::find_if(myStages.rbegin(), myStages.rend()
		, [scene_name](const shared_ptr<Scene>& scene) {
		return (0 == std::strcmp(scene_name, scene->myName.c_str()));
	});

	if (it != myStageIterator)
	{
		myStageIterator = it;
		myState = *myStageIterator;
		myState->Start();
	}

	return bool(scene);
}

void Framework::Connect(const char* ip)
{
	myNetwork.Start(ip);
	myNetwork.Update();
}

PID Framework::GetMyID() const
{
	return myNetwork.myProfile.myID;
}

void Framework::Awake()
{
	if (0 < myScenes.size())
	{
		for (auto& scene : myScenes)
		{
			scene.second->Awake();
		}
	}
}

void Framework::Start()
{
	if (0 < myStages.size())
	{
		std::reverse(myStages.begin(), myStages.end());
		myStageIterator = myStages.rbegin();

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
			JumpToNextScene();
		}

		auto packet = GetLastTask();
		if (packet && myState->OnNetwork(*packet))
		{
			PopTask();

			// ???? ?? ???? ?????? ???? ?????? ?????? ????
			//delete packet;
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

void Framework::OnNetwork(std::vector<Packet*>&& packets)
{
	for (auto& packet : packets)
	{
		AddTask(std::forward<Packet*>(packet));
	}
}

void Framework::OnMouse(UINT type, WPARAM button, LPARAM cursor)
{
	if (myState)
	{
		myState->OnMouse(type, button, cursor);
	}
}

void Framework::OnKeyboard(UINT type, WPARAM key, LPARAM states)
{
	if (myState)
	{
		myState->OnKeyboard(type, key, states);
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

void Framework::Register(const shared_ptr<Scene>& scene)
{
	myScenes.try_emplace(scene->myName, scene);
}

void Framework::Register(shared_ptr<Scene>&& scene)
{
	const auto&& my_scene = std::forward<shared_ptr<Scene>>(scene);
	myScenes.try_emplace(my_scene->myName, my_scene);
}

void Framework::AddTask(Packet* packet)
{
	myTasks.insert(myTasks.begin(), packet);
}

Packet* Framework::GetLastTask() const
{
	if (0 < myTasks.size())
		return myTasks.back();
	else
		return nullptr;
}

void Framework::PopTask()
{
	myTasks.pop_back();
}

shared_ptr<Scene> Framework::Push(Scene* scene)
{
	auto ptr = shared_ptr<Scene>(scene);

	Register(ptr);
	myStages.push_back(ptr);

	return ptr;
}

shared_ptr<Scene> Framework::Pop()
{
	return *(myStageIterator++);
}

shared_ptr<Scene> Framework::GetScene(const char* name) const
{
	return myScenes.at(name);
}
