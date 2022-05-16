#pragma once
#include "Network.hpp"

class Framework
{
public:
	Framework(Network& network);
	~Framework();

	void AddRoom(Scene* scene);
	bool JumpToNextScene();
	bool JumpToPrevScene();
	bool JumpTo(const char* scene_name);
	void Connect(const char* ip);

	void Awake();
	void Start();
	void Update(float elapsed_time);
	void BeginRendering(HWND hwnd);
	void Render(HDC surface);

	bool IsPaused() const;
	bool TryPause();
	void Pause();
	void Resume();

	void OnNetwork(const Packet& packet);
	void OnMouse(UINT type, WPARAM button, LPARAM cursor);
	void OnKeyboard(UINT type, WPARAM key, LPARAM states);
	void OnWindow(WPARAM aevent, LPARAM params);

private:
	void Register(const shared_ptr<Scene>& scene);
	void Register(shared_ptr<Scene>&& scene);

	shared_ptr<Scene> Push(Scene* scene);
	shared_ptr<Scene> Pop();
	shared_ptr<Scene> GetScene(const char* name) const;

	Network& myNetwork;

	std::unordered_map<string, shared_ptr<Scene>> myScenes;
	std::vector<shared_ptr<Scene>> myPipeline;
	std::vector<shared_ptr<Scene>>::reverse_iterator myPipelineIterator;
	shared_ptr<Scene> myState;

	bool isPaused;

	HDC appSurface;
	PAINTSTRUCT appPainter;
};
