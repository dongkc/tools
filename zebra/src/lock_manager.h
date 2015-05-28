#ifndef LOCK_MANAGER_H
#define LOCK_MANAGER_H
#include <string>
#include <memory>
#include <fstream>
#include <boost/pool/object_pool.hpp>
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>
#include <boost/atomic.hpp>

#include "BaseDefine.h"
#include "threadsafe_queue.h"
#include "boost/threadpool.hpp"
#include "lock_request_parser.h"
#include "lock_response_generator.h"
#include "CommandMessageListener.h"
#include "EventMessageSender.h"

class CJT808Server;

class LockManager

{
public:
  LockManager();

  ~LockManager();

  void init();

  void start();

  void stop();

  static int process_request(FrameMsg* frame);

  void process_response(const std::string& message);

  void dispatch(const std::string& message, const std::string& phone_num);

  static std::string generate_auth_code(std::string& phone_num);

private:
  void receive();

  void process_messages(EventMessageSender* sender, int index);

  void poll(int index);

private:
  boost::threadpool::pool thread_pool;
  threadsafe_queue<std::string> response_queue;
  boost::atomic<bool> is_stop;
  std::unique_ptr<CommandMessageListener> mq_listener;
  std::unique_ptr<EventMessageSender> mq_sender;
  std::vector<std::unique_ptr<boost::thread>> workers;
  std::vector<threadsafe_queue<std::string>> request_queues;
};

int process_request(FrameMsg* frame, CJT808Server*, unsigned short);

int SaveRawFile(HANDLE out, unsigned char *buf, UINT len);
#endif
