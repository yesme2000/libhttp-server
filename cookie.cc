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
#include <string>
#include <ctime>
#include <iomanip>

namespace http
{
namespace server
{
using std::string;

string
URLEncode(string input, bool skip_spaces)
{
	string ret;

	for (unsigned char c : input)
	{
		if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
				(c >= '0' && c < '9') || c == '/' ||
			       	c == '.' || (skip_spaces && c == ' '))
			ret.push_back(c);
		else
		{
			ret.push_back('%');
			if (c >= 0xA0)
				ret.push_back('A' + ((c - 0xA0) >> 4));
			else
				ret.push_back('0' + (c >> 4));

			if ((c & 0x0F) >= 0x0A)
				ret.push_back('A' + ((c & 0x0F) - 0x0A));
			else
				ret.push_back('0' + (c & 0x0F));
		}
	}

	return ret;
}

Cookie::Cookie()
: expires(0), max_age(0), rfc(0), version(1), port(0), discard(false),
       	http_only(false), secure(false)
{
}

Cookie::~Cookie()
{
}

string
Cookie::ToString() const
{
	string rv = URLEncode(name) + "=\"" + URLEncode(value, true) + "\"";

	if (expires)
	{
		char buf[25];
		std::time_t exp_c =
		       	std::chrono::system_clock::to_time_t(*expires);
		if (std::strftime(buf, 100, "%a, %d-%b-%Y %H:%M:%S",
				       	std::gmtime(&exp_c)))
	       	{
			rv += "; Expires=" + string(buf) + " GMT";
		}
	}
	if (max_age > 0)
		rv += "; Max-Age=" + std::to_string(max_age);
	if (discard)
		rv += "; Discard";
	if (path.length() > 0)
		rv += "; Path=" + URLEncode(path, true);
	if (domain.length() > 0)
		rv += "; Domain=" + URLEncode(domain, true);
	if (port > 0)
		rv += "; Port=" + std::to_string(port);
	if (secure)
		rv += "; Secure";
	if (version > 0)
		rv += "; Version=" + std::to_string(version);
	if (comment_url.length() > 0)
		rv += "; CommentURL=\"" + URLEncode(comment_url, true) + "\"";

	return rv;
}
}  // namespace server
}  // namespace http
