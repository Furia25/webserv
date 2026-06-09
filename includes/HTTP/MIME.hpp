/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MIME.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/03 14:19:43 by vdurand           #+#    #+#             */
/*   Updated: 2026/06/09 19:08:05 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef _MIME_H
# define _MIME_H

# include "EnumClass.hpp"
# include <map>

# define MIME_TYPE (\
	(txt, text/plain), \
	(html, text/html), \
	(css, text/css), \
	(js, text/javascript), \
	(mjs, application/javascript), \
	(ecma, application/ecmascript), \
	(json, application/json), \
	(jsonld, application/ld+json), \
	(xml, application/xml), \
	(rss, application/rss+xml), \
	(atom, application/atom+xml), \
	(pdf, application/pdf), \
	(doc, application/msword), \
	(docx, application/vnd.openxmlformats-officedocument.wordprocessingml.document), \
	(xls, application/vnd.ms-excel), \
	(xlsx, application/vnd.openxmlformats-officedocument.spreadsheetml.sheet), \
	(ppt, application/vnd.ms-powerpoint), \
	(pptx, application/vnd.openxmlformats-officedocument.presentationml.presentation), \
	(odt, application/vnd.oasis.opendocument.text), \
	(ods, application/vnd.oasis.opendocument.spreadsheet), \
	(odp, application/vnd.oasis.opendocument.presentation), \
	(zip, application/zip), \
	(tar, application/x-tar), \
	(gz, application/gzip), \
	(bz2, application/x-bzip2), \
	(sevenz, application/x-7z-compressed), \
	(rar, application/x-rar-compressed), \
	(bin, application/octet-stream), \
	(swf, application/x-shockwave-flash), \
	(xul, application/vnd.mozilla.xul+xml), \
	(mpkg, application/vnd.apple.installer+xml), \
	(form, application/x-www-form-urlencoded), \
	(multipart, multipart/form-data), \
	(csv, text/csv), \
	(tsv, text/tsv), \
	(md, text/markdown), \
	(ics, text/calendar), \
	(vcf, text/vcard), \
	(jpeg, image/jpeg), \
	(jpg, image/jpeg), \
	(png, image/png), \
	(gif, image/gif), \
	(webp, image/webp), \
	(svg, image/svg+xml), \
	(bmp, image/bmp), \
	(tiff, image/tiff), \
	(heic, image/heic), \
	(heif, image/heif), \
	(ico, image/vnd.microsoft.icon), \
	(mp3, audio/mpeg), \
	(ogg, audio/ogg), \
	(wav, audio/wav), \
	(weba, audio/webm), \
	(aac, audio/aac), \
	(flac, audio/flac), \
	(m4a, audio/mp4), \
	(mp4, video/mp4), \
	(webm, video/webm), \
	(ogv, video/ogg), \
	(mov, video/quicktime), \
	(avi, video/x-msvideo), \
	(wmv, video/x-ms-wmv), \
	(woff, font/woff), \
	(woff2, font/woff2), \
	(ttf, font/ttf), \
	(otf, font/otf), \
	(ttc, font/collection)\
) \

# define X(tuple, ...)	_M_TUPLE_ELEM_0 tuple __VA_ARGS__
# define X_STRING(tuple, ...)	_M_TUPLE_ELEM_1 tuple __VA_ARGS__
ENUM_CLASS(MIME, MIME_TYPE, X, ENUM_LITERALS(MIME_TYPE, X, X_STRING);
public:
	static MIME from_extension(const char *str)
	{
		if (str == NULL)
			ENUM_CLASS_STRING_ERROR("NULL");

		size_t				start = (str[0] == '.') ? 1 : 0;
		const char*			key = str + start;
		static RadixTree<E>	ext_tree;
		static bool			ext_init = false;

		if (!ext_init)
		{
			#define _X_INSERT(el, ...) ext_tree.insert(M_STR(X(el)), X(el));
			M_TUPLE_FOREACH(MIME_TYPE, _X_INSERT)

			ext_init = true;
		}
		RadixTree<E>::const_iterator it = ext_tree.find(key);
		if (it != ext_tree.end())
			return it->second;
		return MIME::bin;
	}

	static const char *get_extension(MIME type)
	{
		if (type == MIME::sevenz)
			return "7z";
		switch (type)
		{
			#define _X_SWITCH(el, ...) case X(el): return M_STR(X(el));
			M_TUPLE_FOREACH(MIME_TYPE, _X_SWITCH)
		}
		return "bin";
	};

	static MIME from_extension(const std::string& str)
	{
		if (str.empty())
			return MIME(MIME::bin);
		return from_extension(str.c_str());
	}
);

# undef X
# undef X_STRING
# undef MIME_TYPE
#undef _ENUM_CLASS_EXT_INSERT


#endif // _MIME_H