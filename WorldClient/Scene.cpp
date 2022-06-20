#include "pch.hpp"
#include "Scene.hpp"
#include "Framework.hpp"
#include "GameCamera.hpp"

Scene::Scene(Framework& framework, const char* name, size_t instance_count)
	: myFramework(framework), myName(name)
	, myCamera()
	, myInstances()
	, isCompleted(false), isPaused(false), isConsistent(false)
{
	myInstances.reserve(instance_count);
}

Scene::~Scene()
{
	if (!isConsistent)
	{
		Reset();
	}
}

void Scene::DestroyInstance(GameObject* instance)
{
	if (0 < myInstances.size() && instance)
	{
		auto it = std::find_if(myInstances.begin(), myInstances.end()
			, [instance](const shared_ptr<GameObject>& ptr) {
			return (ptr.get() == instance);
		});

		if (myInstances.end() != it)
		{
			myInstances.erase(it);
		}
	}
}

void Scene::Awake()
{
	if (0 < myInstances.size())
	{
		for (auto& instance : myInstances)
		{
			instance->Awake();
		}
	}
}

void Scene::Start()
{
	if (0 < myInstances.size())
	{
		for (auto& instance : myInstances)
		{
			instance->Start();
		}
	}
}

void Scene::Update(float time_elapsed)
{
	if (0 < myInstances.size() && 0.0f < time_elapsed)
	{
		for (auto& instance : myInstances)
		{
			instance->Update(time_elapsed);
		}
	}
}

void Scene::Render(HDC surface)
{
	if (0 < myInstances.size())
	{
		const auto& my_cam_pos = myCamera.myPosition;

		for (auto& instance : myInstances)
		{
			instance->Render(surface, -my_cam_pos[0], -my_cam_pos[1]);
		}
	}
}

void Scene::Reset()
{
	isCompleted = false;
}

void Scene::Complete()
{
	isCompleted = true;
}

void Scene::UpdateCamera(const GameObject* follower)
{
	const auto& follower_pos = follower->myPosition;

	myCamera.myPosition[0] = std::max(std::min(follower_pos[0] - FRAME_W / 2, WORLD_W - FRAME_W / 2), 0.0f);
	myCamera.myPosition[1] = std::max(std::min(follower_pos[1] - FRAME_H / 2, WORLD_H - FRAME_H / 2), 0.0f);
}

void Scene::AddInstance(GameObject* instance)
{
	myInstances.push_back(shared_ptr<GameObject>(instance));
}

bool Scene::TryPause()
{
	if (!IsPaused())
	{
		Pause();

		return true;
	}
	return false;
}

void Scene::Pause()
{
	isPaused = true;
}

void Scene::Resume()
{
	isPaused = false;
}

bool Scene::OnNetwork(const Packet& packet)
{
	return false;
}

void Scene::OnMouse(UINT type, WPARAM button, LPARAM cursor)
{}

void Scene::OnKeyboard(UINT type, WPARAM key, LPARAM states)
{
	switch (type)
	{
		case WM_KEYDOWN:
		{
			OnKeyDown(key, states);
		}
		break;

		case WM_KEYUP:
		{
			OnKeyUp(key, states);
		}
		break;
	}
}

void Scene::OnKeyDown(WPARAM key, LPARAM states)
{}

void Scene::OnKeyUp(WPARAM key, LPARAM states)
{}

void Scene::OnWindow(WPARAM aevent, LPARAM params)
{}

bool Scene::IsCompleted() const
{
	return isCompleted;
}

bool Scene::IsPaused() const
{
	return isPaused;
}
