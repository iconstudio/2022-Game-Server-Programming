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

	[[nodiscard]] bool OnNetwork(Packet* packet) override;
	void OnKeyDown(WPARAM key, LPARAM states) override;

private:
	string streamIP;

	const int draw_x = CLIENT_W / 2;
	const int draw_y = CLIENT_H / 2;
	const RECT streamRect;
};

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

	[[nodiscard]] bool OnNetwork(Packet* packet) override;

private:
	const int draw_x = CLIENT_W / 2;
	const int draw_y = CLIENT_H / 2 - 70;
	const RECT streamRect;

	float myLife;
};

class SceneGame : public Scene
{
public:
	SceneGame(Framework& framework);

	void Awake() override;
	void Start() override;
	void Update(float time_elapsed) override;
	void Render(HDC surface) override;
	void Reset() override;
	void Complete() override;

	[[nodiscard]] bool OnNetwork(Packet* packet) override;
	void OnKeyDown(WPARAM key, LPARAM states) override;

private:
	const int sightRange = 5;
	int view_x, view_y;

	// 시야에 보이는 객체 목록 (시야)
	std::vector<GameEntity*> myLocalInstances;
	// 저장해 둔 고유 객체 캐시
	concurrren<GameEntity*> myLocalCached;
};
