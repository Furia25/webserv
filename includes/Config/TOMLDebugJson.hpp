/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TOMLDebugJson.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/14 01:39:40 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/14 17:36:43 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _TOML_JSON_H
# define _TOML_JSON_H
# include <sstream>
# include <string>
# include "TOMLDocument.hpp"

namespace toml
{

static std::string escapeJsonString(const std::string& s)
{
    std::ostringstream out;
    for (std::string::const_iterator it = s.begin(); it != s.end(); ++it)
    {
        unsigned char c = *it;
        switch (c)
        {
            case '"':  out << "\\\""; break;
            case '\\': out << "\\\\"; break;
            case '\n': out << "\\n";  break;
            case '\r': out << "\\r";  break;
            case '\t': out << "\\t";  break;
            default:
                if (c < 0x20)
                {
                    out << "\\u00";
                    out << "0123456789abcdef"[c >> 4];
                    out << "0123456789abcdef"[c & 0xf];
                }
                else
                    out << c;
                break;
        }
    }
    return out.str();
}

static std::string valueToJson(const toml::Value& val, int indent = 0)
{
    std::ostringstream out;
    std::string pad(indent * 2, ' ');
    std::string inner((indent + 1) * 2, ' ');

    if (val.isExplicit())
        out << " ¤ ";

    switch (val.getType())
    {
        case toml::Value::TABLE:
        {
            const toml::Table& table = val.as<toml::Table>();
            out << "{\n";
            bool first = true;
            for (toml::Table::const_iterator it = table.begin(); it != table.end(); ++it)
            {
                if (!first) out << ",\n";
                first = false;
                out << inner << "\"" << escapeJsonString(it->first) << "\": ";
                out << valueToJson(it->second, indent + 1);
            }
            out << "\n" << pad << "}";
            break;
        }
        case toml::Value::ARRAY:
        {
            const toml::Array& array = val.as<toml::Array>();
            out << "[\n";
            bool first = true;
            for (toml::Array::const_iterator it = array.begin(); it != array.end(); ++it)
            {
                if (!first) out << ",\n";
                first = false;
                out << inner << valueToJson(*it, indent + 1);
            }
            out << "\n" << pad << "]";
            break;
        }
        case toml::Value::STRING:
            out << "\"" << escapeJsonString(val.as<std::string>()) << "\"";
            break;
        case toml::Value::INTEGER:
            out << val.as<long long>();
            break;
        case toml::Value::FLOATING:
        {
            double d = val.as<double>();
            if (d != d)             // NaN — invalide en JSON
                out << "null";
            else if (d == d + 1)    // inf
                out << (d > 0 ? "1e999" : "-1e999");
            else
            {
                std::ostringstream ss;
                ss.precision(17);
                ss << d;
                std::string s = ss.str();
                // Garantir qu'il y a un point décimal pour distinguer des entiers
                if (s.find('.') == std::string::npos && s.find('e') == std::string::npos)
                    s += ".0";
                out << s;
            }
            break;
        }
        case toml::Value::BOOLEAN:
            out << (val.as<bool>() ? "true" : "false");
            break;
        default:
            out << "null";
            break;
    }
    return out.str();
}

static std::string toJson(toml::Document& doc)
{
    return valueToJson(doc.getRoot());
}

} // namespace toml

#endif // _TOML_JSON_H