/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPHeartbeat.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/12 16:10:16 by vdurand           #+#    #+#             */
/*   Updated: 2026/06/12 17:20:09 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTP/HTTPHandler.hpp"
#include "Server/TCPServer.hpp"
#include <dirent.h>

static size_t get_max_fds(void);
static size_t count_open_fds(void);
static size_t get_rss_kb(void);

void TCPServer::tickCallback(void *instance)
{
	static size_t	last_total_requests = 0;
	static time_t	last_tick_time      = 0;
	static size_t	max_fds             = get_max_fds();
	static pid_t	pid                 = getpid();

	TCPServer	*server		= static_cast<TCPServer *>(instance);
	time_t		now			= time(NULL);
	time_t		uptime		= now - server->startTime;
	time_t		tick_delta	= (last_tick_time > 0) ? (now - last_tick_time) : 1;

	size_t	current_req = server->handler
		? static_cast<HTTPHandler *>(server->handler)->getTotalRequests() : 0;
	double	rps         = tick_delta > 0
		? (double)(current_req - last_total_requests) / tick_delta : 0.0;
	last_total_requests = current_req;
	last_tick_time      = now;

	size_t	open_fds = count_open_fds();
	size_t	conn     = server->actualConnections;
	size_t	mem_kb   = get_rss_kb();

	char	uptime_buf[32];
	snprintf(uptime_buf, sizeof(uptime_buf), "%ldd%02ldh%02ldm%02lds",
		uptime / 86400, (uptime % 86400) / 3600,
		(uptime % 3600) / 60, uptime % 60);

	Logger::INFO()
		<< "[beat]"
		<< " pid="  << pid
		<< " up="   << uptime_buf
		<< " rps="  << std::fixed << std::setprecision(1) << rps
		<< " conn=" << conn << "/" << MAX_CLIENTS
		            << "(" << conn * 100 / MAX_CLIENTS << "%)"
		<< " fds="  << open_fds << "/" << max_fds
		            << "(" << (max_fds > 0 ? open_fds * 100 / max_fds : 0) << "%)"
		<< " mem="  << mem_kb << "kB";
}

static size_t get_max_fds(void)
{
	std::ifstream	f("/proc/self/limits");
	std::string		line;
	size_t			soft = 0;

	if (!f.is_open())
		return 0;

	while (std::getline(f, line))
	{
		if (line.compare(0, 14, "Max open files") == 0)
		{
			std::sscanf(line.c_str(), "Max open files %zu", &soft);
			break;
		}
	}
	return soft;
}

static size_t count_open_fds(void)
{
	DIR		*dir = opendir("/proc/self/fd");
	size_t	count = 0;

	if (!dir)
		return 0;
	while (readdir(dir))
		++count;
	closedir(dir);
	return count > 2 ? count - 2 : 0;
}

static size_t get_rss_kb(void)
{
	std::ifstream	f("/proc/self/status");
	std::string		line;
	size_t			rss = 0;

	if (!f.is_open())
		return 0;

	while (std::getline(f, line))
	{
		if (line.compare(0, 6, "VmRSS:") == 0)
		{
			std::sscanf(line.c_str(), "VmRSS: %zu", &rss);
			break;
		}
	}
	return rss;
}
