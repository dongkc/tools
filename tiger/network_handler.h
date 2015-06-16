#ifndef NETWORK_HANDLER_H
#define NETWORK_HANDLER_H

#include <string>

#include <Poco/NObserver.h>
#include <Poco/Net/SocketReactor.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/SocketNotification.h>

class NetworkHandler
{
public:

  NetworkHandler(Poco::Net::StreamSocket socket, Poco::Net::SocketReactor& reactor);

  void onReadable(const Poco::AutoPtr<Poco::Net::ReadableNotification>& notification);

  void onWritable(const Poco::AutoPtr<Poco::Net::WritableNotification>& notification);

private:
  Poco::Net::StreamSocket _socket;
  Poco::Net::SocketReactor& _reactor;
  std::string _request;
};

#endif
