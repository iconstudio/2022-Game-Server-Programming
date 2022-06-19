#pragma once

enum EVENT_TYPE { EV_MOVE, EV_HEAL, EV_ATTACK };

struct Timeline
{
	PID object_id;
	EVENT_TYPE ev;
	std::chrono::system_clock::time_point act_time;
	PID target_id;

	constexpr bool operator < (const Timeline& _Left) const
	{
		return (act_time > _Left.act_time);
	}
};
