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

#include <QtNetwork/QHostAddress>
#include <QtNetwork/QTcpServer>
#include <QtCore/QString>
#include <QtCore/QList>

namespace http
{
namespace server
{
class Headers;
class Request;

// Representation of the connections peer.
class Peer
{
public:
	QHostAddress peerAddress();
};

// Callback class to receive information from a Protocol implementation.
// FIXME(tonnerre): Perhaps come up with a data structure instead of the SID
// hack.
class ProtocolObserver
{
public:
	// Get a new ID to identify the session, if required. May always
	// return 0 if it is always clear what session will be used.
	virtual int GetNewSid() = 0;

	// Reset all data received from the protocol for the given session,
	// e.g. if a new connection was made.
	virtual void ClearProtocolData(int sid) = 0;

	// Sets the headers for the given connection SID to exactly the value.
	virtual void SetHeader(int sid, Headers new_headers) = 0;

	// Adds the given header to the currently known set (for protocols with
	// incremental headers).
	virtual void AddHeader(int sid, Headers incremental_headers) = 0;

	// Removes the given headers from the currently known set.
	virtual void RemoveHeader(int sid, Headers decremental_headers) = 0;

	// Indicates that a new request has been recevied.
	virtual void ProcessRequest(int sid, Request* req) = 0;
};

// Wire protocol decoder class.
class Protocol
{
public:
	// Create a protocol parser for HTTP and return it.
	static Protocol* HTTP();

	// Create a protocol parser for HTTPS and return it.
	static Protocol* HTTPS();

	// Indicates whether or not the socket should be initialized as TLS for
	// this connection.
	virtual bool WantsTLS() = 0;

	// Instruct the protocol decoder to start decoding the data on the
	// socket.
	virtual void DecodeConnection(QAbstractSocket* sock) = 0;
};

// A regular HTTP/SPDY/? header. Can contain multiple values.
class Header
{
public:
	// Construct a new header object with the given key and list of values.
	Header(QString key, QList<QString> values);

	// Set the name of the header to something else.
	void SetName(QString newkey);

	// Get the name the header is currently known under.
	QString GetName() const;

	// Add a new value to the header object. It will be appended to the end
	// of the value list.
	void AddValue(QString value);

	// Remove the value from the list of values for the header. If there
	// is no such value in the list, this does nothing.
	void DeleteValue(QString value);

	// Removes all values for the header.
	void ClearValues();

	// Retrieve the first value of the header, as an easy accessor.
	QString GetFirstValue() const;

	// Retrieve all values which are found in the header.
	QList<QString> GetValues() const;

	// Merge the two header objects by adding all values from other to
	// this vector. If the keys of the two objects differ (and the key of
	// other is not empty), this will return false and do nothing.
	bool Merge(const Header& other);

	// Compare the two header objects by looking at their header names.
	bool operator<(const Header& other) const;
	bool operator==(const Header& other) const;

private:
	QString key_;
	QList<QString> values_;
};

// Collection of header lines.
class Headers
{
public:
	// Create a new empty header list.
	Headers();

	// Add value as a new header value for key.
	void Add(QString key, QString value);

	// Replace all values recorded for key with value.
	void Set(QString key, QString value);

	// Remove all values recorded for key, if any.
	void Delete(QString key);

	// Get all values associated with key.
	Header* Get(QString key);

private:
	QMap<QString, Header> headers_;
};

// Backchannel for responses back to the client.
class ResponseWriter
{
	// TODO(tonnerre): Add.
public:
	// Add headers to return to the requester.
	virtual void AddHeaders(const Headers& to_add) = 0;

	// Write the response out with the given status code.
	virtual void WriteHeader(int status_code) = 0;

	// Write the given data to the HTTP/SPDY/? connection. Unlike
	// WriteHeader, this can be called repeatedly. If WriteHeader hasn't
	// been called, it is invoked with an status 200 (OK).
	virtual int Write(QByteArray data) = 0;
};

struct Cookie
{
	QString name;
	QString value;
	QString path;
	QString domain;
	unsigned long expires;
	unsigned long max_age;
	bool secure;
	bool http_only;
	QString raw;

	QString ToString() const;
};

// HTTP/SPDY/? request object.
class Request
{
	// TODO(tonnerre): Add.
public:
	void AddCookie(Cookie* c);
	QList<Cookie*> GetCookies();
	QString FirstFormValue(QString key);
	bool ProtoAtLeast(int major, int minor);
	QString Referer();
	QString UserAgent();
	void SetBasicAuth(QString username, QString password);
};

// Prototype for a handler for requests.
class Handler
{
public:
	// Serve the request "req" writing the response out to "w".
	// The details are left to the implementor.
	virtual void ServeHTTP(const ResponseWriter& w, Request* req) = 0;
};

// The actual HTTP server.
class Server
{
public:
	// Handle all requests to a regexp matching pattern using handler.
	void Handle(QString pattern, Handler* handler);

	// Listen and serve using the protocol decoder proto on the address
	// addr.
	void ListenAndServe(QString addr, Protocol* proto);

	// Serve as the protocol proto on the TCP service srv.
	void Serve(QTcpServer* srv, Protocol* proto);
};
}  // namespace server
}  // namespace http
