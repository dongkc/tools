#include <memory>
#include <glog/logging.h>
#include "EventMessageSender.h"

using namespace std;
using namespace cms;

  //:brokerURI("tcp://172.18.27.154:61616"),
EventMessageSender::EventMessageSender()
  :brokerURI("tcp://192.168.27.13:61616"),
   useTopic(true),
   sessionTransacted(false),
   destName("nuctech.responseTopic"),
   timeToLive(-1)
{
}

EventMessageSender::EventMessageSender(const std::string& brokerURI,
                                       bool useTopic,
                                       bool sessionTransacted )
  :brokerURI(brokerURI),
   useTopic(useTopic),
   sessionTransacted(sessionTransacted),
   destName("nuctech.eventTopic"),
   timeToLive(-1)
{
}

void EventMessageSender::setBrokerURI( const std::string brokerURI )
{
  this->brokerURI = brokerURI;
}

void EventMessageSender::setDestination( bool useTopic, std::string destName )
{
  this->useTopic = useTopic;
  this->destName = destName;
}

void EventMessageSender::setClientID( const std::string clientID )
{
  this->clientID = clientID;
}

void EventMessageSender::setTimeLiveValue( int timeToLive )
{
  this->timeToLive = timeToLive;
}

bool EventMessageSender::start()
{
  bool bResult = false;

  try {
    // Create a ConnectionFactory
    std::auto_ptr<cms::ConnectionFactory> connectionFactory(
      cms::ConnectionFactory::createCMSConnectionFactory( brokerURI ) );

    // Create a Connection
    connection = connectionFactory->createConnection();
    if( !clientID.empty() )
      connection->setClientID(clientID);

    // Create a Session
    if( this->sessionTransacted ) {
      session = connection->createSession( Session::SESSION_TRANSACTED );
    } else {
      session = connection->createSession( Session::AUTO_ACKNOWLEDGE );
    }

    // Create the destination (Topic or Queue)
    if( useTopic ) {
      destination = session->createTopic( destName );
    } else {
      destination = session->createQueue( destName );
    }

    // Create a MessageProducer from the Session to the Topic or Queue
    producer = session->createProducer( destination );
    producer->setDeliveryMode( DeliveryMode::PERSISTENT );
    if (timeToLive != -1)
      producer->setTimeToLive( timeToLive );

    bResult = true;
  }catch ( CMSException& e ) {
    LOG(ERROR) << "destination error";

    release();
  }catch( activemq::exceptions::ActiveMQException& e ) {
    LOG(ERROR) << "destination error";

    release();
  }

  return bResult;
}

void EventMessageSender::close()
{
  this->release();
}

void EventMessageSender::release()
{
  //*************************************************
  // Always close destination, consumers and producers before
  // you destroy their sessions and connection.
  //*************************************************
  try {
    if( destination != NULL ) delete destination;
  } catch (CMSException& e) {
    LOG(ERROR) << "destination error";
  }
	destination = NULL;

  try {
    if( producer != NULL ) delete producer;
  } catch (CMSException& e) {
    LOG(ERROR) << "destination error";
  }
	producer = NULL;

  try {
    if( session != NULL ) session->close();
    if( connection != NULL ) connection->close();
  } catch (CMSException& e) {
    LOG(ERROR) << "destination error";
  }

  try {
     if( session != NULL ) delete session;
  } catch (CMSException& e) {
    LOG(ERROR) << "destination error";
  }
	session = NULL;

	try{
		if( connection != NULL ) delete connection;
	}catch ( CMSException& e ) {
    LOG(ERROR) << "destination error";
	}
	connection = NULL;
}

bool EventMessageSender::sendMessage(const string& data, const string& id)
{
  TextMessage* message = NULL;
  bool bResult = false;

  try {
    message = session->createTextMessage();

    message->setStringProperty( "clientId", id);
    message->setText(data);

    producer->send(message);

    delete message;
    bResult = true;
  }catch ( CMSException& e ) {
    LOG(ERROR) << "destination error";
  }

  return bResult;
}
