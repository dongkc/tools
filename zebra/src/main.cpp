#include <iostream>
#include <boost/thread.hpp>
#include <glog/logging.h>
#include "EventMessageSender.h"
#include "CommandMessageListener.h"

using namespace std;

int main ()
{
  activemq::library::ActiveMQCPP::initializeLibrary();
  CommandMessageListener mq_listener;
  mq_listener.start();

  EventMessageSender mq_sender;
  mq_sender.start();

  string message("[{\"console\":{\"cmd\":\"get\",\"entry\":\"seals\",\"value\":[\"567891\",\"222222\",\"333333\",\"444444\",\"555555\",\"666666\",\"211111\",\"322222\",\"433333\",\"544444\",\"655555\",\"766666\"]},\"id\":\"8900\",\"phone_num\":\"014799252865\",\"seq_num\":\"1\"}]");
  //string message("[{\"console\":{\"cmd\":\"get\",\"entry\":\"ICNumber\",\"value\":[\"13311111\",\"22222222\",\"33333333\",\"44444444\",\"55555555\"]},\"id\":\"8900\",\"phone_num\":\"014799252865\",\"seq_num\":\"1\"}]");
  LOG(INFO) << "MESSAGE: " << message;
  mq_sender.sendMessage(message, "localhost");

  boost::this_thread::sleep(boost::posix_time::seconds(60000));

  activemq::library::ActiveMQCPP::shutdownLibrary();
  return 0;
}
