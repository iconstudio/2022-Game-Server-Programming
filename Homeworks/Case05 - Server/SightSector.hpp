#pragma once

class SightSector
{
public:
	SightSector(int x, int y, float w, float h);
	~SightSector();

	void Acquire();
	void Release();
	bool TryAcquire();

	void Add(const PID id);
	void Remove(const PID id);

	std::vector<PID> GetSightList() const;

	bool operator==(const SightSector& other) const noexcept;

	const int index_x, index_y;

private:
	std::atomic_flag isOwned;

	concurrent_set<PID> mySight;
};
