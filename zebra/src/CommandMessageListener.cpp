#include <memory>
#include <queue>
#include <glog/logging.h>
#include "CommandMessageListener.h"

using namespace std;
using namespace cms;

CommandMessageListener::CommandMessageListener()
  :
   brokerURI("tcp://192.168.27.13:61616"),
   useTopic(true),
   sessionTransacted(false),
   destName("nuctech.gpsSeventTopic"),
   started(false)
{
   //destName("nuctech.commandTopic"),
}

CommandMessageListener::CommandMessageListener(
                         const std::string& brokerURI,
                         bool useTopic,
                         bool sessionTransacted )
  :
   brokerURI(brokerURI),
   useTopic(useTopic),
   sessionTransacted(sessionTransacted),
   destName("nuctech.eventTopic"),
   started(false)
{
   //destName("nuctech.commandTopic"),
}

void CommandMessageListener::setBrokerURI( const std::string brokerURI )
{
  this->brokerURI = brokerURI;
}

void CommandMessageListener::setDestination( bool useTopic, std::string destName )
{
  this->useTopic = useTopic;
  this->destName = destName;
}

void CommandMessageListener::setClientID( const std::string clientID )
{
  this->clientID = clientID;
}

void CommandMessageListener::setDurableName( const std::string durableName )
{
  this->durable = durableName;
}

void CommandMessageListener::setSelector( const std::string selector )
{
  this->selector = selector;
}

bool CommandMessageListener::start()
{
  bool bResult = false;

  try{
    // Create a ConnectionFactory
    auto_ptr<ConnectionFactory> connectionFactory(
      ConnectionFactory::createCMSConnectionFactory( brokerURI ) );

    // Create a Connection
    connection = connectionFactory->createConnection();
    if( !clientID.empty() )
      connection->setClientID(clientID);
    connection->start();
    connection->setExceptionListener(this);

    // Create a Session
    if( this->sessionTransacted == true ) {
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

    // Create a MessageConsumer from the Session to the Topic or Queue
    if( durable.empty() ) {
      consumer = session->createConsumer( destination, selector );
    } else {
      consumer = session->createDurableConsumer( (Topic*)destination, durable, selector );
    }
    consumer->setMessageListener( this );

    bResult = true;
  }catch( CMSException& e ) {
    LOG(ERROR) << "start failed";

    release();
  }catch( activemq::exceptions::ActiveMQException& e ) {

    LOG(ERROR) << "start failed 2";
    release();
  }

  return bResult;
}

void CommandMessageListener::close()
{
  try{
    if( started && connection != NULL ) {
      connection->stop();
      started = false;
    }
  }catch (CMSException& e) {
    LOG(ERROR) << "start failed 3";
  }

  this->release();
}

void CommandMessageListener::startListener()
{
  try{
    if( connection != NULL ) {
      connection->start();
      started = true;
    }
  }catch (CMSException& e) {
    LOG(ERROR) << "start failed 4";
  }
}

void CommandMessageListener::stopListener()
{
  try{
    if( connection != NULL ) {
      connection->stop();
      started = false;
    }
  }catch (CMSException& e) {
    LOG(ERROR) << "start failed 5";
  }
}

void CommandMessageListener::onMessage( const Message* message )
{
  try{
    const TextMessage* text_message = dynamic_cast< const TextMessage* >( message );

    if( text_message != NULL ) {
      LOG(INFO) << text_message->getText();

    }
  } catch (CMSException& e) {
    LOG(ERROR) << "send message error";
  }

  // Commit all messages.
  if( this->sessionTransacted ) {
    session->commit();
  }
}

void CommandMessageListener::onException( const CMSException& ex AMQCPP_UNUSED )
{
}

void CommandMessageListener::release()
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

  try {
    if( consumer != NULL ) delete consumer;
  } catch (CMSException& e) {
    LOG(ERROR) << "destination error";
  }

  try {
    if( session != NULL ) session->close();
    if( connection != NULL ) connection->close();
    session->close();
  } catch (CMSException& e) {
    LOG(ERROR) << "destination error";
  }

  try {
     if( session != NULL ) delete session;
     if( connection != NULL ) delete connection;
  } catch (CMSException& e) {
    LOG(ERROR) << "destination error";
  }
}
