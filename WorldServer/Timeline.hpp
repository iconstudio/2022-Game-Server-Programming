#pragma once

class Timeline
{
public:
	PID object_id;
	OVERLAP_OPS ev = OVERLAP_OPS::NONE;

	std::chrono::system_clock::time_point act_time;
	PID target_id;

	constexpr bool operator < (const Timeline& _Left) const
	{
		return (act_time > _Left.act_time);
	}
};
