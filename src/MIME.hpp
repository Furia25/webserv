/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MIME.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/03 14:19:43 by vdurand           #+#    #+#             */
/*   Updated: 2026/02/03 16:32:55 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef _MIME_H
# define _MIME_H

# include <string>
# include <ostream>

# define MIME_TYPE \
	X(txt, text/plain) \
	X(html, text/html) \
	X(css, text/css) \
	X(js, text/javascript) \
	X(mjs, application/javascript) \
	X(ecma, application/ecmascript) \
	X(json, application/json) \
	X(jsonld, application/ld+json) \
	X(xml, application/xml) \
	X(rss, application/rss+xml) \
	X(atom, application/atom+xml) \
	X(pdf, application/pdf) \
	X(doc, application/msword) \
	X(docx, application/vnd.openxmlformats-officedocument.wordprocessingml.document) \
	X(xls, application/vnd.ms-excel) \
	X(xlsx, application/vnd.openxmlformats-officedocument.spreadsheetml.sheet) \
	X(ppt, application/vnd.ms-powerpoint) \
	X(pptx, application/vnd.openxmlformats-officedocument.presentationml.presentation) \
	X(odt, application/vnd.oasis.opendocument.text) \
	X(ods, application/vnd.oasis.opendocument.spreadsheet) \
	X(odp, application/vnd.oasis.opendocument.presentation) \
	X(zip, application/zip) \
	X(tar, application/x-tar) \
	X(gz, application/gzip) \
	X(bz2, application/x-bzip2) \
	X(sevenz, application/x-7z-compressed) \
	X(rar, application/x-rar-compressed) \
	X(bin, application/octet-stream) \
	X(swf, application/x-shockwave-flash) \
	X(xul, application/vnd.mozilla.xul+xml) \
	X(mpkg, application/vnd.apple.installer+xml) \
	X(form, application/x-www-form-urlencoded) \
	X(multipart, multipart/form-data) \
	X(csv, text/csv) \
	X(tsv, text/tsv) \
	X(md, text/markdown) \
	X(ics, text/calendar) \
	X(vcf, text/vcard) \
	X(jpeg, image/jpeg) \
	X(jpg, image/jpeg) \
	X(png, image/png) \
	X(gif, image/gif) \
	X(webp, image/webp) \
	X(svg, image/svg+xml) \
	X(bmp, image/bmp) \
	X(tiff, image/tiff) \
	X(heic, image/heic) \
	X(heif, image/heif) \
	X(ico, image/vnd.microsoft.icon) \
	X(mp3, audio/mpeg) \
	X(ogg, audio/ogg) \
	X(wav, audio/wav) \
	X(weba, audio/webm) \
	X(aac, audio/aac) \
	X(flac, audio/flac) \
	X(m4a, audio/mp4) \
	X(mp4, video/mp4) \
	X(webm, video/webm) \
	X(ogv, video/ogg) \
	X(mov, video/quicktime) \
	X(avi, video/x-msvideo) \
	X(wmv, video/x-ms-wmv) \
	X(woff, font/woff) \
	X(woff2, font/woff2) \
	X(ttf, font/ttf) \
	X(otf, font/otf) \
	X(ttc, font/collection) \

struct MIME
{
	
};


#endif // _MIME_H