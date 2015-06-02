/*
 *
 * Connector.cpp
 * -------------
 *
 *Use of the Connector design pattern.
 *
 *Compile with
 *g++ -I /usr/local/poco-1.4.2p1-all/include -L /usr/local/poco-1.4.2p1-all/lib/ -lPocoFoundation -lPocoNet -oConnector Connector.cpp
 *
 **/


#include <cstdlib>
#include <iostream>
#include <string>
#include <Poco/Timespan.h>
#include <Poco/Thread.h>
#include <Poco/NObserver.h>
#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/SocketReactor.h>
#include <Poco/Net/SocketConnector.h>
#include <Poco/Net/SocketNotification.h>


using std::cout;
using std::endl;
using std::string;
using Poco::AutoPtr;
using Poco::Timespan;
using Poco::Thread;
using Poco::NObserver;
using Poco::Net::SocketAddress;
using Poco::Net::StreamSocket;
using Poco::Net::SocketReactor;
using Poco::Net::SocketConnector;
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

  void onWritable(const AutoPtr<WritableNotification>& notification)
  {
    string request = "Hello, World!";
    _socket.sendBytes(request.c_str(), request.length());
  }

  void onReadable(const AutoPtr<ReadableNotification>& notification)
  {
    char buf[100];
    int no = _socket.receiveBytes(buf, 99);
    if (no > 0)
    {
      buf[no] = '\0';
      cout << "ServiceHandler::onReadable(): buf=" << buf << endl;
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

  private:

  StreamSocket _socket;

  SocketReactor& _reactor;
};


class ServiceInitializer
{
public:

  ServiceInitializer(StreamSocket socket, SocketReactor& reactor) : _socket(socket), _reactor(reactor)
  {
    cout << "ServiceInitializer::ServiceInitializer():" << endl;
    ServiceHandler* handler = new ServiceHandler(_socket, _reactor);
    _reactor.addEventHandler(_socket, NObserver<ServiceHandler, WritableNotification>(*handler, &ServiceHandler::onWritable));
    _reactor.addEventHandler(_socket, NObserver<ServiceHandler, ReadableNotification>(*handler, &ServiceHandler::onReadable));
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
  SocketAddress address("localhost:9000");
  SocketReactor reactor(Timespan(0, 0, 0, 1, 0));
  SocketConnector<ServiceInitializer> connector(address, reactor);

  Thread thread;
  thread.start(reactor);
  thread.join();

  return EXIT_SUCCESS;
}

