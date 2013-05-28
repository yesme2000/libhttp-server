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

#include <siot/acknowledgementdecorator.h>
#include <siot/connection.h>
#include <siot/server.h>
#include <string>
#include <thread++/mutex.h>
#include <thread++/threadpool.h>
#include <toolbox/expvar.h>
#include <toolbox/scopedptr.h>

#include "server.h"
#include "server_internal.h"

namespace http
{
namespace server
{
using google::protobuf::Closure;
using google::protobuf::NewCallback;
using std::map;
using std::string;
using std::unique_lock;
using threadpp::Mutex;
using threadpp::MutexLock;
using threadpp::ThreadPool;
using toolbox::ExpMap;
using toolbox::ExpVar;
using toolbox::siot::AcknowledgementDecorator;
using toolbox::siot::Connection;
using toolbox::siot::Server;

static ExpVar<int64_t> numConnections("http-server-num-connections");
static ExpVar<int64_t> numOpenConnections("http-server-open-connections");
static ExpMap<int64_t> clientConnectionErrors("http-server-client-connection-errors");

class TCPPeer : public Peer
{
public:
	TCPPeer(Protocol* proto, Connection* sock)
	: proto_(proto), sock_(sock)
	{
	}

	virtual Protocol* PeerProtocol() const
	{
		return proto_;
	}

	virtual string PeerAddress() const
	{
		return sock_->PeerAsText();
	}

	virtual Connection* PeerSocket() const
	{
		return sock_;
	}

	virtual Server* Parent() const
	{
		return sock_->GetServer();
	}

private:
	Protocol* const proto_;
	Connection* const sock_;
};

WebServer::WebServer()
: multiplexer_(new ServeMux), executor_lock_(Mutex::Create()),
	num_threads_(10), idle_timeout_(180), shutdown_(false)
{
}

WebServer::~WebServer()
{
}

void
WebServer::Handle(const string& pattern, Handler* handler)
{
	multiplexer_->Handle(pattern, handler);
}

void
WebServer::ListenAndServe(const string& addr, Protocol* protocol)
{
	Server srv(addr, 0, num_threads_);
	if (protocol->WantsTLS())
		srv.SetServerSSLContext(protocol->GetContext());
	if (idle_timeout_ > 0)
		srv.SetMaxIdle(idle_timeout_);
	Serve(&srv, protocol);
}

void
WebServer::Serve(Server* srv, Protocol* proto)
{
	shutdown_ = false;
	MutexLock lk(executor_lock_.Get());

	if (executor_.IsNull())
		executor_.Reset(new ThreadPool(num_threads_));

	srv->SetConnectionCallback(new ProtocolServer(this, proto,
				multiplexer_.Get()));
	servers_.push_back(srv);
	srv->Listen();
}

void
WebServer::SetExecutor(ThreadPool* executor)
{
	MutexLock lk(executor_lock_.Get());
	executor_.Reset(executor);
}

void
WebServer::SetNumThreads(uint32_t num_threads)
{
	num_threads_ = num_threads;
}

void
WebServer::SetIdleTimeout(int timeout)
{
	idle_timeout_ = timeout;
}

void
WebServer::Shutdown()
{
	for (Server* srv : servers_)
		srv->Shutdown();
}

ThreadPool*
WebServer::GetExecutor()
{
	return executor_.Get();
}

void
WebServer::ServeConnection(const Peer* peer)
{
	Protocol* proto = peer->PeerProtocol();
	proto->DecodeConnection(executor_.Get(), multiplexer_.Get(), peer);
}

ProtocolServer::ProtocolServer(WebServer* parent, Protocol* proto,
	       	ServeMux* mux)
: parent_(parent), proto_(proto), multiplexer_(mux)
{
}

ProtocolServer::~ProtocolServer()
{
}

Connection*
ProtocolServer::AddDecorators(Connection* in)
{
	return new AcknowledgementDecorator(in, 10485760);
}

void
ProtocolServer::DataReady(Connection* conn)
{
	TCPPeer peer(proto_, conn);
	if (!conn->TryReadLock())
		return;
	try
	{
		proto_->DecodeConnection(parent_->GetExecutor(),
				multiplexer_, &peer);
	}
	catch (toolbox::siot::ClientConnectionException ex)
	{
		clientConnectionErrors.Add(ex.identifier(), 1);
		conn->DeferredShutdown();
	}
	conn->Unlock();
}

void
ProtocolServer::ConnectionEstablished(Connection* conn)
{
	numConnections.Add(1);
	numOpenConnections.Add(1);
}

void
ProtocolServer::ConnectionTerminated(Connection* conn)
{
	numOpenConnections.Add(-1);
}

void
ProtocolServer::Error(Connection* conn)
{
	clientConnectionErrors.Add("poll-error", 1);
}

void
ProtocolServer::ConnectionFailed(string msg)
{
	clientConnectionErrors.Add(msg, 1);
}

Protocol::~Protocol()
{
}

Peer::~Peer()
{
}
}  // namespace server
}  // namespace http
