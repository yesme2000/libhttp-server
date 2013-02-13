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

#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QtCore/QScopedPointer>
#include <QtCore/QString>
#include <thread++/threadpool.h>
#include "server.h"

using threadpp::ThreadPool;
using google::protobuf::Closure;
using google::protobuf::NewCallback;

namespace http
{
namespace server
{
class TCPPeer : public Peer
{
public:
	TCPPeer(Protocol* proto, const QHostAddress& address,
			QTcpSocket* sock, QTcpServer* parent)
	: proto_(proto), address_(address), sock_(sock), parent_(parent)
	{
	}

	virtual Protocol* PeerProtocol()
	{
		return proto_;
	}

	virtual QHostAddress PeerAddress()
	{
		return address_;
	}

	virtual QAbstractSocket* PeerSocket()
	{
		return sock_;
	}

	virtual QTcpServer* Parent()
	{
		return parent_;
	}

private:
	Protocol* proto_;
	QHostAddress address_;
	QTcpSocket* sock_;
	QTcpServer* parent_;
};

Server::Server()
: num_threads_(10), shutdown_(false)
{
}

void
Server::Handle(QString pattern, Handler* handler)
{
	multiplexer_.Handle(pattern, handler);
}

void
Server::ListenAndServe(QString addr, Protocol* protocol)
{
	int colon = addr.lastIndexOf(':');
	QHostAddress l_addr;
	qint16 port;

	if (colon == -1)
	{
		l_addr = QHostAddress::Any;
		port = addr.toShort();
	}
	else
	{
		l_addr = QHostAddress(addr.left(colon-1));
		port = addr.right(addr.length() - colon).toShort();
	}

	QTcpServer srv;
	srv.listen(l_addr, port);
	Serve(&srv, protocol);
}

void
Server::Serve(QTcpServer* srv, Protocol* proto)
{
	shutdown_ = false;
	QMutexLocker lk(&executor_lock_);

	if (executor_.isNull())
		executor_.reset(new ThreadPool(num_threads_));

	while (!shutdown_)
	{
		while (srv->hasPendingConnections())
		{
			QTcpSocket* s = srv->nextPendingConnection();
			Peer* p = new TCPPeer(proto, s->peerAddress(),
					s, srv);
			Closure* c =
				NewCallback(this, &Server::ServeConnection, p);
			executor_->Add(c);
		}

		srv->waitForNewConnection(-1);
	}
}

void
Server::SetExecutor(ThreadPool* executor)
{
	QMutexLocker lk(&executor_lock_);
	executor_.reset(executor);
}

void
Server::SetNumThreads(uint32_t num_threads)
{
	num_threads_ = num_threads;
}

void
Server::Shutdown()
{
	shutdown_ = true;
}

ThreadPool*
Server::GetExecutor()
{
	return executor_.data();
}

void
Server::ServeConnection(Peer* peer)
{
	Protocol* proto = peer->PeerProtocol();
	proto->DecodeConnection(executor_.data(), &multiplexer_, peer);
}
}  // namespace server
}  // namespace http
