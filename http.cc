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

#include <string>
#include <iostream>
#include <siot/linebufferdecorator.h>

#include "server.h"
#include "server_internal.h"

namespace http
{
namespace server
{
using std::string;
using toolbox::siot::LineBufferDecorator;

class HTTProtocol : public Protocol
{
public:
	HTTProtocol();
	virtual ~HTTProtocol();

	// Implements Protocol.
	virtual bool WantsTLS();
	virtual void DecodeConnection(threadpp::ThreadPool* executor,
			const ServeMux* mux, const Peer* peer);
};

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

void
HTTProtocol::DecodeConnection(threadpp::ThreadPool* executor,
		const ServeMux* mux, const Peer* peer)
{
	LineBufferDecorator reader(peer->PeerSocket());
	HTTPResponseWriter rw(peer->PeerSocket());
	Request req;
	string command_str = reader.Receive();
	size_t pos, lpos;
	Headers* hdr = new Headers;
	string command;
	string path;
	string protocol;

	pos = command_str.find(' ');
	lpos = command_str.rfind(' ');
	if (pos == string::npos || lpos == string::npos || pos == lpos)
	{
		ScopedPtr<Handler> err =
			Handler::ErrorHandler(400, "Invalid Request");
		err->ServeHTTP(&rw, &req);
		delete hdr;
		return;
	}
	req.SetProtocol(command_str.substr(lpos));
	req.SetAction(command_str.substr(0, pos - 1));
	req.SetPath(command_str.substr(pos + 1, lpos - pos - 1));

	string line = reader.Receive();

	while (line.length() > 0 && line != "\n")
	{
		size_t offset = line.find(':');
		string key = line.substr(0, offset);

		while (line[++offset] == ' ' && offset < line.length());

		string value = line.substr(offset, line.length() - offset - 1);
		hdr->Add(key, value);
		line = reader.Receive();
	}

	req.SetHeaders(hdr);
}

Protocol*
Protocol::HTTP()
{
	return new HTTProtocol();
}
}  // namespace server
}  // namespace http
