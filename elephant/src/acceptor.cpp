/*
 *
 * Acceptor.cpp
 * ------------
 *
 *Use of the Acceptor design pattern.
 *
 *Compile with
 *g++ -I /usr/local/poco-1.4.2p1-all/include/ -L /usr/local/poco-1.4.2p1-all/lib -lPocoFoundation -lPocoNet -oAcceptor Acceptor.cpp
 *
 **/


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


using std::cout;
using std::endl;
using std::string;
using Poco::AutoPtr;
using Poco::Timespan;
using Poco::Thread;
using Poco::NObserver;
using Poco::Net::SocketReactor;
using Poco::Net::ServerSocket;
using Poco::Net::StreamSocket;
using Poco::Net::SocketAcceptor;
using Poco::Net::SocketNotification;
using Poco::Net::ReadableNotification;
using Poco::Net::WritableNotification;


// handles communication and service logic
class ServiceHandler
{
public:

  ServiceHandler(StreamSocket socket, SocketReactor& reactor) : _socket(socket), _reactor(reactor)
  {
  }

  void onReadable(const AutoPtr<ReadableNotification>& notification)
  {
    char buf[100];
    int no = _socket.receiveBytes(buf, 99);
    if (no > 0)
    {
      buf[no] = '\0';
      cout << "ServiceHandler::onReadable(): buf=" << buf << endl;
      _request = buf;
    }
    else
    {
      // destroy handlers
      cout << "ServiceHandler::onReadable(): destroying handlers" << endl;
      _reactor.removeEventHandler(_socket, NObserver<ServiceHandler, ReadableNotification>(*this, &ServiceHandler::onReadable));
      _reactor.removeEventHandler(_socket, NObserver<ServiceHandler, WritableNotification>(*this, &ServiceHandler::onWritable));
      _socket.close();
      delete this;
    }
  }

  void onWritable(const AutoPtr<WritableNotification>& notification)
  {
    if (!_request.empty())
    {
      cout << "ServiceHandler::onWritable(): _request=" << _request << endl;
      _socket.sendBytes(_request.c_str(), _request.length());
      _request.clear();
    }
  }

  private:

  StreamSocket _socket;

  SocketReactor& _reactor;

  string _request;
};


class ServiceInitializer
{
public:

  ServiceInitializer(StreamSocket& socket, SocketReactor& reactor) : _socket(socket), _reactor(reactor)
  {
    cout << "ServiceInitializer::ServiceInitializer(): client accepted" << endl;
    ServiceHandler* handler = new ServiceHandler(_socket, _reactor);
    _reactor.addEventHandler(_socket, NObserver<ServiceHandler, ReadableNotification>(*handler, &ServiceHandler::onReadable));
    _reactor.addEventHandler(_socket, NObserver<ServiceHandler, WritableNotification>(*handler, &ServiceHandler::onWritable));
  }

  ~ServiceInitializer()
  {
    cout << "ServiceInitializer::~ServiceInitializer():" << endl;
  }

  private:

  StreamSocket _socket;

  SocketReactor& _reactor;
};


int main()
{
  ServerSocket socket(9000);
  SocketReactor reactor(Timespan(0, 0, 0, 1, 0));
  SocketAcceptor<ServiceInitializer> acceptor(socket, reactor);

  Thread thread;
  thread.start(reactor);
  thread.join();

  return EXIT_SUCCESS;
}
