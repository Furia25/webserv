/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AlarmManager.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/21 13:51:20 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/24 17:32:13 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _ALARMMANAGER_H
# define _ALARMMANAGER_H

# include <stdint.h>

typedef uint64_t	AlarmID;

class Alarm
{
public:
	Alarm();
	Alarm(const Alarm& other);
	~Alarm();
	Alarm& operator=(const Alarm& other);
protected:
private:
};

class TimeWheel
{
public:
protected:
private:
};

class AlarmManager
{
public:
protected:
private:

	

	AlarmManager();
	~AlarmManager();
};

#endif // _ALARMMANAGER_H
