/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestBuilder.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 14:55:27 by antbonin          #+#    #+#             */
/*   Updated: 2026/04/20 14:06:16 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _REQUESTBUILDER_H
# define _REQUESTBUILDER_H

# include <iostream>
# include <vector>
# include <string>
# include <stdint.h>
# include <sstream>
# include <cctype>
# include <exception>
# include <cstdlib>

# include "Utils/HashMap.hpp"
# include "HttpTypes.hpp"
# include "Request.hpp"


struct MethodMap 
{
    const char* str;
    Method val;
};

static const MethodMap map[]= 
{
    { _HTTP_GET_, Method::GET },
    { _HTTP_POST_, Method::POST },
    { _HTTP_DELETE_, Method::DELETE }
};

static const size_t map_size = sizeof(map) / sizeof(map[0]);

class RequestBuilder
{
private:
    std::vector<uint8_t>				raw_buffer;
    bool								parsing_is_complete;
    bool								header_is_parsed;
    bool								is_validated;

    size_t								content_length;
    HashMap<std::string, std::string>	headers;
    std::string							method;
    std::string							request_path;
    std::string							query_path;
    std::string							protocol;
    std::vector<uint8_t>				body;

    size_t		find_newline(const std::vector<uint8_t>& buffer, size_t start, size_t max);
    size_t		find_header_end();
    void		parse_all_headers(const std::vector<uint8_t>& buffer, size_t pos);
    void		parseRequestLine(std::string &line);
    void		parseHeaderLine(std::string &line);
    void		toLowerCase(std::string &str);
    void		validateMethod() const;
    void		validateProtocol() const;
    void		validatePath();
    void		validateHeader();
    void		invalidPath();

public:
    RequestBuilder();
    RequestBuilder(const RequestBuilder &other);
    RequestBuilder &operator=(const RequestBuilder &other);
    ~RequestBuilder();

    void		feed(const uint8_t *fragment, size_t length);
    void		check();

    Request		build() const;

    void		print() const;

    const bool	&getCompleteStatus() const;
    const bool	&isHeaderParsed() const;
    const bool	&isValidated() const;
    void		setValidateStatus(int status);
};

#endif // _REQUESTBUILDER_H