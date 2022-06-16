#pragma once

class Framework
{
public:
	Framework(HWND hwnd);
	~Framework();

	void Awake();
	void Start();
	void Update(float delta_time);
	void Release();

	std::vector<shared_ptr<Sprite>> mySprites;
};
