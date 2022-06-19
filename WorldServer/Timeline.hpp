#pragma once

enum TIMELINE_EVENTS { EV_MOVE, EV_HEAL, EV_ATTACK };

class Timeline
{
public:
	PID object_id;
	TIMELINE_EVENTS ev;

	std::chrono::system_clock::time_point act_time;
	PID target_id;

	constexpr bool operator < (const Timeline& _Left) const
	{
		return (act_time > _Left.act_time);
	}
};
