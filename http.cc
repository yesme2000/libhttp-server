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

#include <algorithm>
#include <string>
#include <siot/acknowledgementdecorator.h>
#include <siot/rangereaderdecorator.h>
#include <toolbox/expvar.h>

#include "server.h"
#include "server_internal.h"

namespace http
{
namespace server
{
using std::string;
using toolbox::ExpMap;
using toolbox::ExpVar;
using toolbox::siot::AcknowledgementDecorator;
using toolbox::siot::RangeReaderDecorator;

class HTTProtocol : public Protocol
{
public:
	HTTProtocol();
	virtual ~HTTProtocol();

	// Implements Protocol.
	virtual bool WantsTLS();
	virtual const ServerSSLContext* GetContext();
	virtual void DecodeConnection(threadpp::ThreadPool* executor,
			const ServeMux* mux, const Peer* peer);
};

class HTTPSProtocol : public HTTProtocol
{
public:
	HTTPSProtocol(const ServerSSLContext* context);
	virtual ~HTTPSProtocol();

	// Implements HTTProtocol.
	virtual bool WantsTLS();
	virtual const ServerSSLContext* GetContext();

private:
	const ServerSSLContext* context_;
};

static ExpVar<int64_t> numHttpRequests("http-server-num-http-requests");
static ExpMap<int64_t> numHttpHostRequests("http-server-http-requests-by-host");
static ExpMap<int64_t> numHttpRequestErrors("http-server-http-request-errors");

HTTProtocol::HTTProtocol()
{
}

HTTProtocol::~HTTProtocol()
{
}

bool
HTTProtocol::WantsTLS()
{
	return false;
}

const ServerSSLContext*
HTTProtocol::GetContext()
{
	return 0;
}

void
HTTProtocol::DecodeConnection(threadpp::ThreadPool* executor,
		const ServeMux* mux, const Peer* peer)
{
	AcknowledgementDecorator* ack =
		static_cast<AcknowledgementDecorator*>(peer->PeerSocket());
	// Only attempt processing if we can own the connection.
	if (!ack->TryReadLock())
		return;

	ack->SetBlocking(true);
	string alldata = ack->Receive();
	ack->SetBlocking(false);

	if (alldata.find("\n\n") == string::npos &&
			alldata.find("\r\n\r\n") == string::npos)
	{
		// Try again later when we have more data.
		ack->Unlock();
		return;
	}

	HTTPResponseWriter rw(peer->PeerSocket());
	Request req;
	list<string> lines;
	size_t pos = 0, lpos = 0;
	Headers* hdr = new Headers;
	string command;
	string path;
	string protocol;

	while (lpos < alldata.length() &&
			(pos = alldata.find("\n", lpos)) != string::npos)
	{
		size_t endpos = alldata[pos-1] == '\r' ? pos-1 : pos;

		if (endpos != lpos)
			lines.push_back(alldata.substr(lpos, endpos - lpos));
		else
			break;

		lpos = pos + 1;
	}
	while (lpos < alldata.length() && (alldata[lpos] == '\r' ||
				alldata[lpos] == '\n'))
		lpos++;
	alldata.clear();
	ack->Acknowledge(lpos);

	numHttpRequests.Add(1);

	if (!lines.size())
	{
		ScopedPtr<Handler> err =
			Handler::ErrorHandler(400, "Invalid Request");
		err->ServeHTTP(&rw, &req);
		numHttpRequestErrors.Add("zero-sized-request", 1);
		delete hdr;
		ack->Unlock();
		// Cut the connection, the peer isn't making any sense.
		ack->DeferredShutdown();
		return;
	}

	string command_str = lines.front();
	lines.pop_front();

	pos = command_str.find(' ');
	lpos = command_str.rfind(' ');
	if (pos == string::npos || lpos == string::npos || pos == lpos)
	{
		ScopedPtr<Handler> err =
			Handler::ErrorHandler(400, "Invalid Request");
		err->ServeHTTP(&rw, &req);
		numHttpRequestErrors.Add("unknown-protocol-header", 1);
		delete hdr;
		ack->Unlock();
		// Cut the connection, the peer isn't making any sense.
		ack->DeferredShutdown();
		return;
	}
	req.SetProtocol(command_str.substr(lpos));
	req.SetAction(command_str.substr(0, pos - 1));
	req.SetPath(command_str.substr(pos + 1, lpos - pos - 1));

	for (string line : lines)
	{
		size_t offset = line.find(':');
		string key = line.substr(0, offset);

		while (line[++offset] == ' ' && offset < line.length());

		string value = line.substr(offset, line.length() - offset);

		if (key == "Cookie")
		{
			size_t prev = 0, pos = -2;

			do
			{
				prev = pos + 2;
				pos = value.find("; ", prev);

				if (pos == string::npos)
					pos = value.length();

				string cookie = value.substr(prev, pos - prev);
				size_t eq = cookie.find('=');
				if (eq != string::npos)
				{
					Cookie* ck = new Cookie;
					ck->name = cookie.substr(0, eq);
					// TODO(tonnerre): decode?
					ck->value = cookie.substr(eq + 1);

					req.AddCookie(ck);
				}
			}
			while (pos < value.length());
		}
		else
			hdr->Add(key, value);
	}

	req.SetHeaders(hdr);

	if (hdr->GetFirst("Content-Length").length() > 0)
	{
		unsigned long length =
			strtoul(hdr->GetFirst("Content-Length").c_str(),
					NULL, 10);

		if (length > 0)
		{
			ack->SetAutoAck(true);
			req.SetRequestBody(new RangeReaderDecorator(ack,
						length));
		}
	}

	if (hdr->GetFirst("Host").length() > 0)
	{
		std::string Host = hdr->GetFirst("Host");
		std::string host;
		host.resize(Host.size());
		std::transform(Host.begin(), Host.end(), host.begin(),
				::tolower);
		numHttpHostRequests.Add(host, 1);
	}
	else
		numHttpHostRequests.Add("unknown", 1);

	Handler* handler = mux->GetHandler(req.Path());
	if (!handler)
	{
		ScopedPtr<Handler> err =
			Handler::ErrorHandler(404, "Not Found");
		err->ServeHTTP(&rw, &req);
		numHttpRequestErrors.Add("no-registered-handler", 1);
		if (hdr->GetFirst("Connection").substr(0, 5) == "close")
			ack->DeferredShutdown();
		ack->Unlock();
		return;
	}

	handler->ServeHTTP(&rw, &req);
	ack->SetAutoAck(false);
	ack->Unlock();
	if (hdr->GetFirst("Connection").substr(0, 5) == "close")
		ack->DeferredShutdown();
}

Protocol*
Protocol::HTTP()
{
	return new HTTProtocol();
}

HTTPSProtocol::HTTPSProtocol(const ServerSSLContext* context)
: context_(context)
{
}

HTTPSProtocol::~HTTPSProtocol()
{
}

bool
HTTPSProtocol::WantsTLS()
{
	return true;
}

const ServerSSLContext*
HTTPSProtocol::GetContext()
{
	return context_;
}

Protocol*
Protocol::HTTPS(const ServerSSLContext* context)
{
	return new HTTPSProtocol(context);
}

}  // namespace server
}  // namespace http
