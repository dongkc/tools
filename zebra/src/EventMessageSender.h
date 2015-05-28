#include <string>

#include "activemq.h"

class EventMessageSender
{
public:
  EventMessageSender();

  EventMessageSender(const std::string& brokerURI,
                     bool useTopic,
                     bool sessionTransacted );

public:
  void setBrokerURI( const std::string brokerURI = "failover:(tcp://localhost:61616)" );

  void setDestination( bool useTopic = true, std::string destName = "nuctech.eventTopic" );

  void setClientID( std::string clientID = "" );

  void setTimeLiveValue( int timeToLive = -1 );

  bool start();

  void close(); 

  bool sendMessage(const std::string& data, const std::string& id);

private:
  void release();

private:
  cms::Connection       *connection;
  cms::Session          *session;
  cms::Destination      *destination;
  cms::MessageProducer  *producer;

  std::string brokerURI;
  bool useTopic;
  bool sessionTransacted;
  std::string clientID;
  std::string destName;
  int timeToLive;
};
