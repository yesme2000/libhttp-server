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

#include <list>
#include <string>
#include "server.h"

namespace http
{
namespace server
{
using std::list;
using std::string;

Header::Header(string key, list<string> values)
: key_(key), values_(values)
{
}

Header::~Header()
{
}

void
Header::SetName(string newkey)
{
	key_ = newkey;
}

string
Header::GetName() const
{
	return key_;
}

void
Header::AddValue(string value)
{
	values_.push_back(value);
}

void
Header::DeleteValue(string value)
{
	values_.remove(value);
}

void
Header::ClearValues()
{
	values_.clear();
}

string
Header::GetFirstValue() const
{
	if (values_.empty())
		return string();
	return values_.front();
}

list<string>
Header::GetValues() const
{
	return values_;
}

bool
Header::Merge(const Header& other)
{
	if (key_.length() == 0)
		key_ = other.key_;

	if (other.key_ != key_ && other.key_.length() > 0)
		return false;

	values_.insert(values_.end(), other.values_.begin(),
		       	other.values_.end());
	return true;
}

bool
Header::operator<(const Header& other) const
{
	return key_ < other.key_;
}

bool
Header::operator==(const Header& other) const
{
	return key_ == other.key_;
}

Headers::Headers()
{
}

Headers::~Headers()
{
}

void
Headers::Add(string key, string value)
{
	const auto& h = headers_.find(key);
	if (h == headers_.end())
	{
		list<string> values;
		values.push_back(value);
		headers_.insert(std::make_pair(key, Header(key, values)));
	}
	else
	{
		h->second.AddValue(value);
	}
}

void
Headers::Set(string key, string value)
{
	const auto& h = headers_.find(key);
	if (h == headers_.end())
	{
		list<string> values;
		values.push_back(value);
		headers_.insert(std::make_pair(key, Header(key, values)));
	}
	else
	{
		h->second.ClearValues();
		h->second.AddValue(value);
	}
}

void
Headers::Delete(string key)
{
	const auto& h = headers_.find(key);
	if (h != headers_.end())
		headers_.erase(h);
}

Header*
Headers::Get(string key)
{
	const auto& h = headers_.find(key);
	if (h == headers_.end())
		return 0;
	else
		return &h->second;
}
}  // namespace server
}  // namespace http
