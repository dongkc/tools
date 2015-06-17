#include <cstdlib>
#include <iostream>
#include <string>

#include <Poco/Timespan.h>
#include <Poco/Thread.h>
#include <Poco/NObserver.h>
#include <Poco/Net/SocketReactor.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/SocketAcceptor.h>
#include <Poco/Net/SocketNotification.h>

#include "common.h"
#include "net_mgr.h"

using namespace std;
using namespace Poco;
using namespace Poco::Net;

NetworkHandler::NetworkHandler(StreamSocket socket, SocketReactor& reactor) : _socket(socket), _reactor(reactor)
{
  _reactor.addEventHandler(_socket, NObserver<NetworkHandler, ReadableNotification>(*this, &NetworkHandler::onReadable));
  _reactor.addEventHandler(_socket, NObserver<NetworkHandler, WritableNotification>(*this, &NetworkHandler::onWritable));
}

void NetworkHandler::onReadable(const AutoPtr<ReadableNotification>& notification)
{
  char buf[1500];
  int no = _socket.receiveBytes(buf, 1500);
  if (no > 0)
  {
    buf[no] = '\0';
    _request = buf;
  }
  else
  {
    // destroy handlers
    _reactor.removeEventHandler(_socket, NObserver<NetworkHandler, ReadableNotification>(*this, &NetworkHandler::onReadable));
    _reactor.removeEventHandler(_socket, NObserver<NetworkHandler, WritableNotification>(*this, &NetworkHandler::onWritable));
    _socket.close();
    delete this;
  }
}

void NetworkHandler::onWritable(const AutoPtr<WritableNotification>& notification)
{
  if (!_request.empty())
  {
    _socket.sendBytes(_request.c_str(), _request.length());
    _request.clear();
  }
}

#if 0
int main()
{
  ServerSocket socket(9000);
  SocketReactor reactor(Timespan(0, 0, 0, 1, 0));
  SocketAcceptor<NetworkHandler> acceptor(socket, reactor);

  Thread thread;
  thread.start(reactor);
  thread.join();

  return EXIT_SUCCESS;
}
#endif
