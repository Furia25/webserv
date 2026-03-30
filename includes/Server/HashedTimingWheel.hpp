/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HashedTimingWheel.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/21 13:51:20 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/30 19:24:03 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _ALARMMANAGER_H
# define _ALARMMANAGER_H

# include <ctime>
# include <vector>
# include <stdint.h>
# include <sys/time.h>

# define WHEEL_SIZE	8

typedef int64_t timestamp_ms;

#define _HTWTemplate_	template <timestamp_ms resolution>
#define _HTWDef_	HashedTimingWheel<resolution>

static timestamp_ms now_ms()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (timestamp_ms)tv.tv_sec * 1000 + (timestamp_ms)tv.tv_usec / 1000;
}

class AAlarm
{
public:
	AAlarm() : wheel_slot(0), bucket_index(0), expire_at(0), wheel_round(0), scheduled(false) {};
	virtual ~AAlarm() {};
	virtual void	operator()() const = 0;

	bool	is_scheduled(void) { return this->scheduled; };

	_HTWTemplate_
	friend class HashedTimingWheel;
protected:
private:
	size_t		wheel_slot;
	size_t		bucket_index;
	time_t		expire_at;
	uint32_t	wheel_round;
	bool		scheduled;
};

template <typename T>
class Alarm : public AAlarm
{
public:
	typedef void (*callback)(T* ctx);
	Alarm(T& obj, callback cb) : ptr(&obj), cb(cb) {};

	virtual void	operator()() const {if (cb && ptr) {cb(ptr);} } ;
private:
	T				*ptr;
	callback		cb;
};

template <timestamp_ms resolution>
class HashedTimingWheel
{
public:
	HashedTimingWheel();
	~HashedTimingWheel();

	void	schedule(AAlarm& alarm, unsigned int ticks);
	void	cancel(AAlarm& alarm);
	void	reschedule(AAlarm& alarm, unsigned int ticks);

	int		next_timeout_ms() const;
	void	tick(void);
protected:
private:
	size_t					current_bucket;
	timestamp_ms			last_tick;
	std::vector<AAlarm *>	buckets[WHEEL_SIZE];
};

_HTWTemplate_
inline _HTWDef_::HashedTimingWheel() : current_bucket(0), last_tick(now_ms()) {}

_HTWTemplate_
inline _HTWDef_::~HashedTimingWheel() {}

_HTWTemplate_
inline void _HTWDef_::schedule(AAlarm& alarm, unsigned int ticks)
{
	if (alarm.is_scheduled())
		return ;
	if (ticks == 0)
		ticks = 1;

	size_t slot     = (this->current_bucket + ticks) & (WHEEL_SIZE - 1);
	alarm.wheel_round = ticks / WHEEL_SIZE;
	if (ticks % WHEEL_SIZE == 0)
	{
		slot = (this->current_bucket + WHEEL_SIZE) & (WHEEL_SIZE - 1);
		alarm.wheel_round = ticks / WHEEL_SIZE - 1;
	}
	alarm.expire_at = now_ms() + ticks * resolution;
	alarm.wheel_slot = slot;
	alarm.bucket_index = this->buckets[slot].size();
	alarm.scheduled = true;
	this->buckets[slot].push_back(&alarm);
}

_HTWTemplate_
inline void _HTWDef_::cancel(AAlarm& alarm)
{
	if (!alarm.is_scheduled())
		return ;

	std::vector<AAlarm *>&	bucket = this->buckets[alarm.wheel_slot];
	AAlarm	*last = bucket.back();
	bucket[alarm.bucket_index] = last;
	last->bucket_index = alarm.bucket_index;
	bucket.pop_back();

	alarm.scheduled = false;
}

_HTWTemplate_
inline void _HTWDef_::reschedule(AAlarm& alarm, unsigned int ticks)
{
	this->cancel(alarm);
	this->schedule(alarm, ticks);
}

_HTWTemplate_
inline int _HTWDef_::next_timeout_ms() const
{
	timestamp_ms	elapsed	= now_ms() - this->last_tick;
	int				remain	= resolution - static_cast<int>(elapsed);
	return remain > 0 ? remain : 0;
}

_HTWTemplate_
inline void _HTWDef_::tick(void)
{
	timestamp_ms	now = now_ms();
	int				elapsed = (now - this->last_tick) / resolution;

	if (elapsed <= 0)
		return ;
	this->last_tick += elapsed * resolution;
	while (elapsed > 0)
	{
		current_bucket = (current_bucket + 1) & (WHEEL_SIZE - 1);
		std::vector<AAlarm *>& slot = this->buckets[current_bucket];

		for (size_t i = 0; i < slot.size(); i++)
		{
			AAlarm	*alarm = slot[i];
			if (alarm->wheel_round > 0)
			{
				alarm->wheel_round--;
				continue ;
			}

			alarm->scheduled = false;
			alarm->operator()();
			slot[i] = slot.back();
			slot[i]->bucket_index = i;
			slot.pop_back();
			--i;
		}
		elapsed--;
	}
}

#endif // _ALARMMANAGER_H
