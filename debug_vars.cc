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

#include <string>
#include <toolbox/expvar.h>
#include <toolbox/qsingleton.h>

#include "server.h"
#include "server_internal.h"
#include "debug_vars.h"

namespace http
{
namespace server
{
using std::string;
using toolbox::_private::ExpvarRegistry;
using toolbox::ExpVarBase;
using toolbox::QSingleton;

DebugVarsHandler::DebugVarsHandler()
{
}

DebugVarsHandler::~DebugVarsHandler()
{
}

void
DebugVarsHandler::ServeHTTP(ResponseWriter* w, const Request* req)
{
	ExpvarRegistry registry = QSingleton<ExpvarRegistry>::GetInstance();
	Headers h;
	h.Add("Content-type", "application/json; charset=utf-8");

	w->AddHeaders(h);
	w->WriteHeader(200, "OK");

	w->Write("{\r\n");

	// Write the names and values of all debug var keys out to the
	// requester.
	for (string name : registry.Keys())
	{
		ExpVarBase* var = registry.Lookup(name);

		if (var)
			// TODO(tonnerre): Escape properly.
			w->Write("\"" + name + "\": " + var->String() +
					",\r\n");
	}

	w->Write("}\r\n");
}
}  // namespace server
}  // namespace http
