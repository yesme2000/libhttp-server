/*-
 * Copyright (c) 2012 Tonnerre Lombard <tonnerre@ancient-solutions.com>,
 *                    Ancient Solutions. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions  of source code must retain  the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions  in   binary  form  must   reproduce  the  above
 *    copyright  notice, this  list  of conditions  and the  following
 *    disclaimer in the  documentation and/or other materials provided
 *    with the distribution.
 *
 * THIS  SOFTWARE IS  PROVIDED BY  ANCIENT SOLUTIONS  AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO,  THE IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS
 * FOR A  PARTICULAR PURPOSE  ARE DISCLAIMED.  IN  NO EVENT  SHALL THE
 * FOUNDATION  OR CONTRIBUTORS  BE  LIABLE FOR  ANY DIRECT,  INDIRECT,
 * INCIDENTAL,   SPECIAL,    EXEMPLARY,   OR   CONSEQUENTIAL   DAMAGES
 * (INCLUDING, BUT NOT LIMITED  TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE,  DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT  LIABILITY,  OR  TORT  (INCLUDING NEGLIGENCE  OR  OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "server.h"
#include <QtCore/QUrl>

namespace http
{
namespace server
{
static QByteArray exclude = QByteArray(" /");

Cookie::Cookie()
: max_age(0), rfc(0), version(1), port(0), discard(false), http_only(false),
	secure(false)
{
}

QString
Cookie::ToString() const
{
	QString rv = QUrl::toPercentEncoding(name) + "=\"" +
		QUrl::toPercentEncoding(value, exclude) + "\"";

	if (!expires.isNull())
		rv += "; Expires=" +
		       	expires.toUTC().toString("ddd, dd-MMM-yyyy hh:mm:ss") +
			" GMT";
	if (max_age > 0)
		rv += "; Max-Age=" + QString::number(max_age);
	if (discard)
		rv += "; Discard";
	if (path.length() > 0)
		rv += "; Path=" + QUrl::toPercentEncoding(path, exclude);
	if (domain.length() > 0)
		rv += "; Domain=" + QUrl::toPercentEncoding(domain, exclude);
	if (port > 0)
		rv += "; Port=" + QString::number(port);
	if (secure)
		rv += "; Secure";
	if (version > 0)
		rv += "; Version=" + QString::number(version);
	if (comment_url.length() > 0)
		rv += "; CommentURL=\"" +
		       	QUrl::toPercentEncoding(comment_url, exclude) + "\"";

	return rv;
}
}  // namespace server
}  // namespace http
