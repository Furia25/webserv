/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIFactory.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/20 18:48:18 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/20 19:04:43 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "HTTP/CGIFactory.hpp"
# include "HTTP/HTTPTypes.hpp"

size_t	CGIResponseParser::tryParseHeaderLine(const uint8_t* buffer, size_t size, std::string& key, std::string& value)
{
	for (size_t i = 0; i + 1 < size; ++i)
	{
		if (buffer[i] != '\r' || buffer[i+1] != '\n')
			continue;
		if (i == 0)
			return 2;
		size_t colon = std::string::npos;
		for (size_t j = 0; j < i; ++j)
		{
			if (buffer[j] == ':')
			{
				colon = j;
				break;
			}
		}
		if (colon == std::string::npos)
			throw std::invalid_argument("Malformed header: missing colon");
		key = std::string(reinterpret_cast<const char*>(buffer), colon);
		value = std::string(reinterpret_cast<const char*>(buffer + colon + 1), i - colon - 1);
		for (size_t j = 0; j < key.size(); ++j)
			key[j] = std::tolower(key[j]);
		size_t start = value.find_first_not_of(" \t");
		if (start != std::string::npos)
			value = value.substr(start);
		return i + 2;
	}
	return 0; 
}

void	CGIResponseParser::feed(const uint8_t* data, size_t len)
{
	if (buffer_size + len > MAX_CGI_BUFFER)
        throw HTTPException(HTTPCode::BAD_REQUEST);
	std::memcpy(buffer, data, len);
	buffer_size += len;
	size_t pos = 0;
	while (!header_done && pos < buffer_size)
	{
		if (pos + 1 < buffer_size && buffer[pos] == '\r' && buffer[pos+1] == '\n')
		{
			header_done = true;
			pos += 2;
			break;
		}
		std::string key;
		std::string value;
		size_t consumed = tryParseHeaderLine(buffer + pos, buffer_size - pos, key, value);
		if (consumed == 0)
			break;
		if (!first_line_done)
			first_line_done = true;
		if (!key.empty())
		headers.insert(key, value);
		pos += consumed;
	}
	if (pos > 0)
	{
		std::memmove(buffer, buffer + pos, buffer_size - pos);
		buffer_size -= pos;
	}
}

bool	CGIResponseParser::isHeaderParsed()const
{
	return this->header_done;
}