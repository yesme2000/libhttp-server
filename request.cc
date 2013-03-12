/*-
 * Copyright (c) 2013 Tonnerre Lombard <tonnerre@ancient-solutions.com>,
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

#include <list>
#include <string>
#include <utility>

#include <iostream>

#include <toolbox/crypto/base64.h>

#include "server.h"
#include "server_internal.h"

namespace http
{
namespace server
{
using toolbox::Base64;
using std::list;
using std::pair;
using std::string;

Request::~Request()
{
	for (map<string, Cookie*>::iterator cookie = cookies_.begin();
			cookie != cookies_.end(); cookie++)
		delete cookie->second;
}

void
Request::AddCookie(Cookie* c)
{
	map<string, Cookie*>::iterator oldcookie =
		cookies_.find(c->domain + ":" + c->name);
	if (oldcookie != cookies_.end())
	{
		delete oldcookie->second;
		cookies_.erase(oldcookie);
	}

	cookies_.insert(std::make_pair(c->domain + ":" + c->name, c));
}

list<Cookie*>
Request::GetCookies() const
{
	list<Cookie*> ret;
	for (std::map<string, Cookie*>::const_iterator it = cookies_.begin();
			it != cookies_.end(); it++)
	{
		ret.push_back(it->second);
	}
	return ret;
}

void
Request::SetHeaders(Headers* headers)
{
	headers_.Reset(headers);
}

Headers*
Request::GetHeaders() const
{
	return headers_.Get();
}

string
Request::FirstFormValue(const string& key) const
{
	/* TODO(tonnerre): Stub! */ return "";
}

bool
Request::ProtoAtLeast(int major, int minor) const
{
	/* TODO(tonnerre): Stub! */ return false;
}

string
Request::Referer() const
{
	if (headers_.IsNull())
		return "";

	const Header* ref = headers_->Get("Referer");
	if (ref)
		return ref->GetFirstValue();

	return "";
}

string
Request::UserAgent() const
{
	if (headers_.IsNull())
		return "";

	const Header* ref = headers_->Get("User-Agent");
	if (ref)
		return ref->GetFirstValue();

	return "";
}

string
Request::Host() const
{
	if (headers_.IsNull())
		return "";

	const Header* ref = headers_->Get("Host");
	if (ref)
		return ref->GetFirstValue();

	return "";
}

void
Request::SetPath(const string& path)
{
	path_ = path;
}

string
Request::Path() const
{
	return path_;
}

void
Request::SetProtocol(const string& protocol)
{
	protocol_ = protocol;
}

string
Request::Protocol() const
{
	return protocol_;
}

void
Request::SetAction(const string& action)
{
	action_ = action;
}

string
Request::Action() const
{
	return action_;
}

void
Request::SetBasicAuth(const string& username, const string& password)
{
	if (headers_.IsNull())
		return;

	headers_->Set("Authorization", "Basic " +
			Base64::Encode(username + ":" + password));
}

pair<string, string>
Request::GetBasicAuth() const
{
	if (headers_.IsNull())
		return std::make_pair("", "");

	const Header* h = headers_->Get("Authorization");
	if (!h)
		return std::make_pair("", "");

	string auth = h->GetFirstValue();
	if (auth.length() == 0)
		return std::make_pair("", "");

	size_t split = auth.find(' ');
	auth = Base64::Decode(auth.substr(split + 1) + "\n");
	if (auth.length() == 0)
		return std::make_pair("", "");

	split = auth.find(':');
	string user = auth.substr(0, split);
	string pass = auth.substr(split+1);

	return std::make_pair(user, pass);
}

string
Request::AsURL() const
{
	return "http://" + Host() + Path();
}
}  // namespace server
}  // namespace http
