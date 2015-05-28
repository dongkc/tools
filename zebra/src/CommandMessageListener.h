#include <string>
#include <queue>

#include "activemq.h"

class CommandMessageListener : public cms::ExceptionListener,
                             public cms::MessageListener
{
public:
  CommandMessageListener();

  CommandMessageListener(
                         const std::string& brokerURI,
                         bool useTopic = true,
                         bool sessionTransacted = false );

public:

  void setBrokerURI( const std::string brokerURI = "failover:(tcp://localhost:61616)" );

  void setDestination( bool useTopic = true, std::string destName = "nuctech.commandTopic" );

  void setClientID( std::string clientID = "" );

  void setDurableName( std::string durableName = "" );

  void setSelector( std::string selector = "" );

  bool start();

  void close();

  void startListener();

  void stopListener();

  virtual void onMessage( const cms::Message* message );

  virtual void onException( const cms::CMSException& ex);

private:
  void release();

private:
  cms::Connection      *connection;
  cms::Session         *session;
  cms::MessageConsumer *consumer;

public:
  cms::Destination     *destination;

private:
  std::string brokerURI;
  bool useTopic;
  bool sessionTransacted;
  std::string clientID;
  std::string destName;
  std::string durable;
  std::string selector;

  bool started;
};
