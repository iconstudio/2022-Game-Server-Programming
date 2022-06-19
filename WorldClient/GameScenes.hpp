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

	// �� �÷��̾� ĳ����
	PlayerCharacter* myPlayerCharacter;
	// �þ߿� ���̴� ��ü ��� (�þ�)
	std::unordered_map<PID, GameEntity*> myLocalInstances;

	int Orientation = 0;

	// ���� ��Ʈ��
	HBITMAP myWorldImage;
	HDC myWorldImageContext;

	// ���� ���� �� ����
	default_random_engine myRandomEngine;
	uniform_int_distribution<long> myTerrainRandomizer;
	uniform_real_distribution<float> myRegionRandomizer;

	// ���� (���� �̸�, ���� ��ȣ�� RECT)
	std::unordered_map<std::string, RECT> myWorldRegions;

	// ���� (2000x2000 �迭)
	std::array<std::array<int, WORLD_CELLS_CNT_H>, WORLD_CELLS_CNT_V > myWorldTerrain;
};
