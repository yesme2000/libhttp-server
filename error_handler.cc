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

#include "server.h"
#include "server_internal.h"

namespace http
{
namespace server
{
using std::string;

// Error message with the given error code and string.
class ErrorHandlerImpl : public Handler
{
public:
	ErrorHandlerImpl(int errcode, const string& message);
	virtual ~ErrorHandlerImpl();

	// Serve an error message with the given error code and string.
	virtual void ServeHTTP(ResponseWriter* w, const Request* req);

private:
	int code_;
	string message_;
};

Handler::~Handler()
{
}

ErrorHandlerImpl::ErrorHandlerImpl(int errcode, const string& message)
: code_(errcode), message_(message)
{
}

ErrorHandlerImpl::~ErrorHandlerImpl()
{
}

void
ErrorHandlerImpl::ServeHTTP(ResponseWriter* w, const Request* req)
{
	Headers h;
	h.Add("Content-type", "text/plain");
	h.Add("Content-Length", std::to_string(message_.length() + 4));

	w->AddHeaders(h);
	w->WriteHeader(code_);
	w->Write(message_ + "\r\n\r\n");
}

Handler*
Handler::ErrorHandler(int errcode, const string& message)
{
	return new ErrorHandlerImpl(errcode, message);
}
}  // namespace server
}  // namespace http
