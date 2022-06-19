#pragma once
#include "Scene.hpp"

class SceneMain : public Scene
{
public:
	SceneMain(Framework& framework);

	void Awake() override;
	void Start() override;
	void Update(float time_elapsed) override;
	void Render(HDC surface) override;
	void Reset() override;
	void Complete() override;

	[[nodiscard]] bool OnNetwork(const Packet& packet) override;
	void OnKeyDown(WPARAM key, LPARAM states) override;

private:
	string streamIP;

	const int draw_x = CLIENT_W / 2;
	const int draw_y = CLIENT_H / 2;
	const RECT streamRect;
};
