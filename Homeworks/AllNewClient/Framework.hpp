#pragma once

class Framework
{
public:
	Framework();
	~Framework();

	void Push(Scene* scene);

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
	void OnMouse(WPARAM button, LPARAM cursor);
	void OnKeyboard(WPARAM key, LPARAM states);
	void OnWindow(WPARAM aevent, LPARAM params);

private:
	void Push(const shared_ptr<Scene>& scene);
	void Push(shared_ptr<Scene>&& scene);
	bool TryPop();
	shared_ptr<Scene> Pop();

	std::vector<shared_ptr<Scene>> myScenes;
	shared_ptr<Scene> myState;

	bool isPaused;

	HDC appSurface;
	PAINTSTRUCT appPainter;
};
