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
#include <sstream>

#include "server.h"
#include "server_internal.h"

namespace http
{
namespace server
{
using toolbox::siot::Connection;
using std::string;

ResponseWriter::~ResponseWriter()
{
}

HTTPResponseWriter::HTTPResponseWriter(Connection* conn)
: conn_(conn), written_(false)
{
}

HTTPResponseWriter::~HTTPResponseWriter()
{
	// If the connection was actually encoded as chunked, we need to
	// send the final 0 byte to indicate the last chunk.
	if (headers_.GetFirst("Transfer-Encoding") == "chunked")
		conn_->Send("0\r\n\r\n");
}

void
HTTPResponseWriter::AddHeaders(const Headers& to_add)
{
	headers_.Merge(to_add);
}

void
HTTPResponseWriter::WriteHeader(int status_code, string message)
{
	if (written_)
		return;

	written_ = true;
	if (!headers_.Get("Content-Length"))
		headers_.Set("Transfer-Encoding", "chunked");
	else if (!headers_.Get("Connection"))
		headers_.Set("Connection", "keep-alive");

	conn_->Send("HTTP/1.1 " + std::to_string(status_code) +
			" " + message + "\r\n");
	for (const string& h_name : headers_.HeaderNames())
	{
		const Header* hdr = headers_.Get(h_name);

		for (const string& value : hdr->GetValues())
			conn_->Send(h_name + ": " + value + "\r\n");
	}

	conn_->Send("\r\n");
}

int
HTTPResponseWriter::Write(string data)
{
	if (!written_)
		WriteHeader(200);

	if (headers_.GetFirst("Transfer-Encoding") == "chunked")
	{
		std::ostringstream oss;
		oss << std::hex << data.length();
		data = oss.str() + "\r\n" + data + "\r\n";
	}

	return conn_->Send(data);
}
}  // namespace server
}  // namespace http
