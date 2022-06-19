#pragma once
#include "Scene.hpp"

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

	[[nodiscard]] bool OnNetwork(const Packet& packet) override;
	void OnKeyDown(WPARAM key, LPARAM states) override;
	void OnKeyUp(WPARAM key, LPARAM states) override;

private:
	const int sightRange = 5;

	// 내 플레이어 캐릭터
	PlayerCharacter* myPlayerCharacter;
	// 시야에 보이는 객체 목록 (시야)
	std::unordered_map<PID, GameEntity*> myLocalInstances;

	int Orientation = 0;

	// 지형 비트맵
	HBITMAP myWorldImage;
	HDC myWorldImageContext;

	// 지형 생성 용 랜덤
	default_random_engine myRandomEngine;
	uniform_int_distribution<long> myTerrainRandomizer;
	uniform_real_distribution<float> myRegionRandomizer;

	// 전장 (지역 이름, 구역 번호의 RECT)
	std::unordered_map<std::string, RECT> myWorldRegions;

	// 지형 (2000x2000 배열)
	std::array<std::array<int, WORLD_CELLS_CNT_H>, WORLD_CELLS_CNT_V > myWorldTerrain;
};
