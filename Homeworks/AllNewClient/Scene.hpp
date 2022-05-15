#pragma once

class Scene
{
public:
	Scene(size_t instance_count);
	virtual ~Scene();

	Scene(const Scene&) = delete;
	Scene& operator=(const Scene&) = delete;
	Scene(Scene&&) = delete;
	Scene& operator=(Scene&&) = delete;

	virtual void Awake() = 0;
	virtual void Start() = 0;
	virtual void Update(float time_elapsed) = 0;
	virtual void Render(HDC surface) = 0;

	virtual void OnNetwork(const Packet& packet);
	virtual void OnMouse(WPARAM button, LPARAM cursor);
	virtual void OnKeyboard(WPARAM key, LPARAM states);
	virtual void OnWindow(WPARAM aevent, LPARAM params);

	bool IsCompleted() const;
	bool IsPaused() const;

	bool TryPause();
	virtual void Pause() = 0;
	virtual void Resume() = 0;

private:
	virtual void Complete() = 0;

	std::vector<shared_ptr<GameObject>> myInstances;

	bool isCompleted;
	bool isPaused;
};
