#include "pch.hpp"
#include "Scene.hpp"
#include "Framework.hpp"
#include "Packet.hpp"

Scene::Scene(Framework& framework, size_t instance_count)
	: myFramework(framework), myInstances(instance_count)
	, isCompleted(false), isPaused(false), isConsistent(false)
{}

Scene::~Scene()
{
	if (!isConsistent)
	{
		Reset();
	}
}

void Scene::Reset()
{}

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

void Scene::OnNetwork(const Packet& packet)
{}

void Scene::OnMouse(WPARAM button, LPARAM cursor)
{}

void Scene::OnKeyboard(WPARAM key, LPARAM states)
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
