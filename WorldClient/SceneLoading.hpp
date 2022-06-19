#pragma once
#include "Scene.hpp"

class SceneLoading : public Scene
{
public:
	SceneLoading(Framework& framework);

	void Awake() override;
	void Start() override;
	void Update(float time_elapsed) override;
	void Render(HDC surface) override;
	void Reset() override;
	void Complete() override;

	[[nodiscard]] bool OnNetwork(const Packet& packet) override;

private:
	const int draw_x = CLIENT_W / 2;
	const int draw_y = CLIENT_H / 2 - 70;
	const RECT streamRect;

	float myLife;
};

