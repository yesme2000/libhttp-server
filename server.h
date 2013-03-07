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
#include <siot/connection.h>
#include <string>
#include <thread++/threadpool.h>
#include <toolbox/scopedptr.h>
#include <siot/server.h>

namespace http
{
namespace server
{
using std::string;
using std::list;
using std::map;
using std::mutex;
using toolbox::ScopedPtr;

using toolbox::siot::Connection;
using toolbox::siot::ConnectionCallback;
using toolbox::siot::Server;

class Headers;
class Protocol;
class Request;
class ServeMux;

// Convert a string to a URL encoded string.
string URLEncode(string input, bool skip_spaces = false);

// Representation of the connections peer.
class Peer
{
public:
	virtual ~Peer();

	virtual Protocol* PeerProtocol() = 0;
	virtual string PeerAddress() = 0;
	virtual Connection* PeerSocket() = 0;
	virtual Server* Parent() = 0;
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

// Wire protocol decoder class.
class Protocol
{
public:
	virtual ~Protocol();

	// Create a protocol parser for HTTP and return it.
	static Protocol* HTTP();

	// Create a protocol parser for HTTPS and return it.
	static Protocol* HTTPS();

	// Indicates whether or not the socket should be initialized as TLS for
	// this connection.
	virtual bool WantsTLS() = 0;

	// Instruct the protocol decoder to start decoding the data on the
	// socket.
	virtual void DecodeConnection(threadpp::ThreadPool* executor,
			ServeMux* mux, Peer* peer) = 0;
};

// A regular HTTP/SPDY/? header. Can contain multiple values.
class Header
{
public:
	// Construct a new header object with the given key and list of values.
	Header(string key, list<string> values);

	virtual ~Header();

	// Set the name of the header to something else.
	void SetName(string newkey);

	// Get the name the header is currently known under.
	string GetName() const;

	// Add a new value to the header object. It will be appended to the end
	// of the value list.
	void AddValue(string value);

	// Remove the value from the list of values for the header. If there
	// is no such value in the list, this does nothing.
	void DeleteValue(string value);

	// Removes all values for the header.
	void ClearValues();

	// Retrieve the first value of the header, as an easy accessor.
	string GetFirstValue() const;

	// Retrieve all values which are found in the header.
	list<string> GetValues() const;

	// Merge the two header objects by adding all values from other to
	// this vector. If the keys of the two objects differ (and the key of
	// other is not empty), this will return false and do nothing.
	bool Merge(const Header& other);

	// Compare the two header objects by looking at their header names.
	bool operator<(const Header& other) const;
	bool operator==(const Header& other) const;

private:
	string key_;
	list<string> values_;
};

// Collection of header lines.
class Headers
{
public:
	// Create a new empty header list.
	Headers();
	virtual ~Headers();

	// Add value as a new header value for key.
	void Add(string key, string value);

	// Replace all values recorded for key with value.
	void Set(string key, string value);

	// Remove all values recorded for key, if any.
	void Delete(string key);

	// Get all values associated with key.
	Header* Get(string key);

private:
	map<string, Header> headers_;
};

// Backchannel for responses back to the client.
class ResponseWriter
{
	// TODO(tonnerre): Add.
public:
	virtual ~ResponseWriter();

	// Add headers to return to the requester.
	virtual void AddHeaders(const Headers& to_add) = 0;

	// Write the response out with the given status code.
	virtual void WriteHeader(int status_code) = 0;

	// Write the given data to the HTTP/SPDY/? connection. Unlike
	// WriteHeader, this can be called repeatedly. If WriteHeader hasn't
	// been called, it is invoked with a status 200 (OK).
	virtual int Write(string data) = 0;
};

struct Cookie
{
	Cookie();
	virtual ~Cookie();

	string name;
	string value;
	string path;
	string domain;
	string comment_url;

	std::chrono::time_point<std::chrono::system_clock,
	       std::chrono::seconds>* expires;

	unsigned long max_age;
	unsigned long rfc;
	unsigned long version;
	unsigned short port;

	bool discard;
	bool http_only;
	bool secure;

	string raw;

	string ToString() const;
};

// HTTP/SPDY/? request object.
class Request
{
	// TODO(tonnerre): Add.
public:
	virtual ~Request();

	void AddCookie(Cookie* c);
	list<Cookie*> GetCookies();
	string FirstFormValue(string key);
	bool ProtoAtLeast(int major, int minor);
	string Referer();
	string UserAgent();
	void SetBasicAuth(string username, string password);
};

// Prototype for a handler for requests.
class Handler
{
public:
	virtual ~Handler();

	// Serve the request "req" writing the response out to "w".
	// The details are left to the implementor.
	virtual void ServeHTTP(const ResponseWriter& w, Request* req) = 0;
};

// Helper class for distributing the requests efficiently to their handlers.
class ServeMux
{
public:
	virtual ~ServeMux();

	// Handle all requests to a regexp matching pattern using handler.
	void Handle(string pattern, Handler* handler);

	// Find the handler for the given URL (closest matching pattern).
	Handler* GetHandler(string URL);

private:
	map<string, Handler*> candidates_;
};

// The actual HTTP server. By default, it runs on a threadpool with 10
// worker threads which it creates internally.
class WebServer
{
public:
	// Set up the data structures for a new web server, but don't start
	// anything yet.
	WebServer();

	virtual ~WebServer();

	// Handle all requests to a regexp matching pattern using handler.
	void Handle(string pattern, Handler* handler);

	// Listen and serve using the protocol decoder proto on the address
	// addr.
	void ListenAndServe(string addr, Protocol* proto);

	// Serve as the protocol proto on the TCP service srv. This method
	// will block forever, waiting for new connections. If you want to
	// run it in a threadpool, please set one up using SetExecutor() and
	// run Serve in it as a closure.
	void Serve(Server* srv, Protocol* proto);

	// Overrides the default executor with a new one. Must be called
	// before Serve() or ListenAndServe(), but not necessarily before
	// Handle(). Passing a zero argument will revert to the default
	// behavior of creating a built-in threadpool. Takes ownership of
	// the threadpool.
	void SetExecutor(threadpp::ThreadPool* executor);

	// Sets the desired number of threads for the pool. After running
	// SetExecutor() with a nonzero arg, Serve() or ListenAndServe(), this
	// will have no effect.
	void SetNumThreads(uint32_t num_threads);

	// Gets the associated threadpool in case something else wants to
	// run in it.
	threadpp::ThreadPool* GetExecutor();

	// Instruct the server to stop accepting connections. Another call
	// to Serve() or ListenAndServe() will make it resume. If you want
	// to wait until all outstanding connections have been terminated,
	// use SetExecutor(0) or call the destructor.
	void Shutdown();

private:
	void ServeConnection(Peer* peer);

	ServeMux multiplexer_;
	mutex executor_lock_;
	ScopedPtr<threadpp::ThreadPool> executor_;
	list<Server*> servers_;
	uint32_t num_threads_;
	bool shutdown_;
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

}  // namespace server
}  // namespace http
