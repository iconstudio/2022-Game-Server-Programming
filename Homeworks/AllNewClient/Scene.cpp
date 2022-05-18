#include "pch.hpp"
#include "stdafx.hpp"
#include "Scene.hpp"
#include "Framework.hpp"
#include "Packet.hpp"

Scene::Scene(Framework& framework, const char* name, size_t instance_count)
	: myFramework(framework), myName(name)
	, myInstances(instance_count)
	, isCompleted(false), isPaused(false), isConsistent(false)
{}

Scene::~Scene()
{
	if (!isConsistent)
	{
		Reset();
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
		for (auto& instance : myInstances)
		{
			instance->Render(surface);
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

bool Scene::OnNetwork(Packet* packet)
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
