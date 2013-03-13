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
 *
 * HTTP/HTTPS/SPDY server implementation as a library.
 */

#include <chrono>
#include <list>
#include <map>
#include <mutex>
#include <regex>
#include <string>

#include <thread++/threadpool.h>
#include <toolbox/scopedptr.h>
#include <siot/connection.h>
#include <siot/server.h>

namespace http
{
namespace server
{
using std::list;
using std::map;
using std::mutex;
using std::regex;
using std::string;

using toolbox::ScopedPtr;
using toolbox::siot::Connection;
using toolbox::siot::ConnectionCallback;
using toolbox::siot::Server;

class Headers;
class Protocol;
class Request;
class ServeMux;

// Representation of the connections peer.
class Peer
{
public:
	virtual ~Peer();

	virtual Protocol* PeerProtocol() const = 0;
	virtual string PeerAddress() const = 0;
	virtual Connection* PeerSocket() const = 0;
	virtual Server* Parent() const = 0;
};

// Callback class to receive information from a Protocol implementation.
// FIXME(tonnerre): Perhaps come up with a data structure instead of the SID
// hack.
class ProtocolObserver
{
public:
	virtual ~ProtocolObserver();

	// Reset all data received from the protocol for the given session,
	// e.g. if a new connection was made.
	virtual void ClearProtocolData(Peer* p) = 0;

	// Sets the headers for the given connection SID to exactly the value.
	virtual void SetHeader(Peer* p, Headers new_headers) = 0;

	// Adds the given header to the currently known set (for protocols with
	// incremental headers).
	virtual void AddHeader(Peer* p, Headers incremental_headers) = 0;

	// Removes the given headers from the currently known set.
	virtual void RemoveHeader(Peer* p, Headers decremental_headers) = 0;

	// Indicates that a new request has been recevied.
	virtual void ProcessRequest(Peer* p, Request* req) = 0;
};

// Helper class for distributing the requests efficiently to their handlers.
class ServeMux
{
public:
	virtual ~ServeMux();

	// Handle all requests to a prefix pattern using handler.
	void Handle(const string& pattern, Handler* handler);

	// Find the handler for the given path (closest matching prefix).
	Handler* GetHandler(const string& path) const;

private:
	map<string, Handler*> candidates_;
};

// An instance of the server taking care of a specific protocol.
class ProtocolServer : public toolbox::siot::ConnectionCallback
{
public:
	ProtocolServer(WebServer* parent, Protocol* proto, ServeMux* mux);
	virtual ~ProtocolServer();

	// Implements ConnectionCallback.
	virtual void ConnectionEstablished(Connection* conn);
	virtual void DataReady(Connection* conn);
	virtual void ConnectionTerminated(Connection* conn);
	virtual void Error(Connection* conn);

private:
	WebServer* parent_;
	Protocol* proto_;
	ServeMux* multiplexer_;
};

class HTTPResponseWriter : public ResponseWriter
{
public:
	// Create a new HTTP response writer for the connection "conn".
	HTTPResponseWriter(Connection* conn);
	virtual ~HTTPResponseWriter();

	// Implements ResponseWriter.
	virtual void AddHeaders(const Headers& to_add);
	virtual void WriteHeader(int status_code, string message = "OK");
	virtual int Write(string data);

private:
	Connection* conn_;
	Headers headers_;
	bool written_;
};

}  // namespace server
}  // namespace http
