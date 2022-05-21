#pragma once

using shared_sight = shared_atomic<SightSector>;

class SightSector
{
public:
	SightSector(int x, int y, float w, float h);
	~SightSector();

	void Add(const shared_ptr<GameEntity>& entity);
	void Add(shared_ptr<GameEntity>&& entity);
	void Remove(const shared_ptr<GameEntity>& entity);
	void Remove(shared_ptr<GameEntity>&& entity);

	bool operator==(const SightSector& other) const noexcept;

	const int index_x, index_y;

private:
	std::vector<weak_ptr<GameEntity>> seeingInstances;
	std::vector<weak_ptr<GameEntity>>::iterator::difference_type seeingLast;
};
