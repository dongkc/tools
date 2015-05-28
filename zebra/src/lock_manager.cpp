#include <string>
#include <deque>
#include <boost/bind.hpp>
#include <boost/make_unique.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/functional/hash.hpp>

#include <Poco/DigestStream.h>
#include <Poco/MD5Engine.h>
#include <Poco/LocalDateTime.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/DateTimeFormat.h>

#include "threadsafe_queue.h"
#include "lock_manager.h"
#include "BaseDefine.h"
#include "ConditionVariableQueue.h"
#include "CacheMgr.h"
#include "config.h"

#include "JT808Server.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "util.h"
#include "log.h"

extern CCacheMgr<FrameMsg> gcachemgr;
extern CConfiguration gconfig;
extern CConditionVariableQueue<FrameMsg> grecvmsglst;
extern CConditionVariableQueue<FrameMsg> gsendmsglst;
extern CJT808Server gserver;

using namespace std;
using rapidjson::Document;
using rapidjson::Value;
using rapidjson::StringBuffer;
using rapidjson::Writer;
using Poco::DigestOutputStream;
using Poco::DigestEngine;
using Poco::MD5Engine;

using Poco::LocalDateTime;
using Poco::DateTimeFormat;
using Poco::DateTimeFormatter;

LockManager::LockManager()
  :is_stop(false)
{
}

LockManager::~LockManager()
{
}

void LockManager::init()
{
  mq_listener.reset(new CommandMessageListener(response_queue,
                                               gconfig.mq_ip,
                                               gconfig.mq_consumer_usetopic,
                                               false));
  mq_listener->setDestination(gconfig.mq_consumer_usetopic, gconfig.mq_consumer_destination);
  mq_listener->setSelector(gconfig.mq_consumer_selector);
  mq_listener->start();

  for (uint8_t i = 0; i < gconfig.mq_producer_num; ++i) {
    std::unique_ptr<boost::thread> worker;
    workers.push_back(boost::move(worker));

    threadsafe_queue<std::string> queue;
    request_queues.push_back(queue);
  }


  thread_pool.size_controller().resize(1);
}

void LockManager::start()
{
  for (uint8_t i = 0; i < workers.size(); ++i) {
    workers[i].reset(new boost::thread(&LockManager::poll, this, i));
  }
}

void LockManager::poll(int index)
{
  EventMessageSender sender(gconfig.mq_ip, gconfig.mq_producer_usetopic, false);
  sender.setDestination(gconfig.mq_producer_usetopic, gconfig.mq_producer_destination);

  sender.start();

  while (!is_stop) {
    receive();
    process_messages(&sender, index);

    boost::this_thread::sleep(boost::posix_time::seconds(1));
  }

  sender.close();
}

void LockManager::stop()
{
  is_stop = true;
  for (uint8_t i = 0; i < workers.size(); ++i) {
    workers[i]->join();
  }

  mq_listener->close();
}

void LockManager::receive()
{
  string message;
  while (response_queue.try_pop(message)) {
    Document document;
    document.Parse(message.c_str());
    if (document.HasParseError()) {
      return;
    }

    if (!document.IsArray()) {
      return;
    }

    const Value& a = document;
    for (rapidjson::SizeType i = 0; i < a.Size(); ++i) {
      const Value& e = a[i];
      StringBuffer buffer;
      Writer<StringBuffer> writer(buffer);
      e.Accept(writer);

      thread_pool.schedule(
        boost::bind(&LockManager::process_response, this, string(buffer.GetString())));
    }
  }
}

void LockManager::process_messages(EventMessageSender* sender, int index)
{
  vector<string> vec;
  string json;
  while (request_queues[index].try_pop(json)) {
    vec.push_back(json);
  }
  if (!vec.empty()) {
    string joined(boost::algorithm::join(vec, ","));
    string array("[" + joined + "]");

    sender->sendMessage(array, gconfig.mq_consumer_selector, mq_listener->destination);
  }
}

void LockManager::process_response(const string& message)
{
  LockResponseGenerator response(message);

  string frame_string(response.generate_frame());
  if (frame_string.empty()) {
    return;
  }

  SOCKET socket = 0;;
  uint32_t sessionid = 0;
  if (!gserver.GetClientConnectInfo(response.get_phone_num(), socket, sessionid)) {
    return;
  }

  FrameMsg* frame = gcachemgr.Get();
  frame->msgtype = FrameMsg::NORMAL;
  frame->fd = socket;
  frame->sessionid = sessionid;
  frame->datalen = frame_string.size();
  memcpy(frame->data, frame_string.data(), frame->datalen);

  if (!gsendmsglst.inQueue(frame)) {
  }
}

void LockManager::dispatch(const std::string& message, const std::string& phone_num)
{
  boost::hash<string> string_hash;
  std::size_t h = string_hash(phone_num);
  request_queues[h%request_queues.size()].push(message);
}

std::string LockManager::generate_auth_code(std::string& phone_num)
{
  MD5Engine md5;
  DigestOutputStream ostr(md5);

  ostr << phone_num;
  ostr << "nuc";    // salt
  ostr.flush();
  return DigestEngine::digestToHex(md5.digest());
}

int process_request(FrameMsg* frame, CJT808Server* server, unsigned short res_seq_num)
{
  LockRequestParser request((char*)frame->data, frame->datalen);

  if (!request.parse_header()) {
	  g_Logger.writeHex("lock_manager.cpp line: ",__LINE__,log4cplus::DEBUG_LOG_LEVEL,L" error frame: ", (const char*)frame->data, frame->datalen);
    return -1;
  };

  string message_id(request.get_message_id());
  string phone_num(request.get_phone_num());
  uint16_t seq_num(request.get_sequence_num());

  string message(request.generate_message());

  string response_frame;
  if (message_id == "0200") {
	  if(server->m_lock_mgr != NULL)
		server->m_lock_mgr->dispatch(message, phone_num);
    response_frame = LockResponseGenerator::generate_general_frame(phone_num, message_id, res_seq_num, seq_num, 0);
  }
  else if (message_id == "0102") {
    server->RegClientId(frame->fd, frame->sessionid, phone_num);
    response_frame = LockResponseGenerator::generate_general_frame(phone_num, message_id, res_seq_num, seq_num,
       strcmp(request.get_auth_code().c_str(), LockManager::generate_auth_code(phone_num).c_str()) == 0 ? 0 : 1);		
#ifdef __TEST__
	SYSTEMTIME curtm = {0};
	GetLocalTime(&curtm);
	printf("%s\t%d-%02d-%02d %02d:%02d:%02d\tauthentication begin\n", GETNULLPTR(phone_num),curtm.wYear,curtm.wMonth,curtm.wDay,
		curtm.wHour,curtm.wMinute,curtm.wSecond);
#endif	
  }
  else if (message_id == "0002") {
    response_frame = LockResponseGenerator::generate_general_frame(phone_num, message_id, res_seq_num, seq_num, 0);
  }
  else if (message_id == "0100") {
    server->RegClientId(frame->fd, frame->sessionid, phone_num);
    response_frame = LockResponseGenerator::generate_register_frame(phone_num, LockManager::generate_auth_code(phone_num), seq_num, 0);
#ifdef __TEST__
	SYSTEMTIME curtm = {0};
	GetLocalTime(&curtm);
	printf("%s\t%d-%02d-%02d %02d:%02d:%02d\tRegister begin\n", GETNULLPTR(phone_num),curtm.wYear,curtm.wMonth,curtm.wDay,
		curtm.wHour,curtm.wMinute,curtm.wSecond);
#endif	
  }

  if (message.empty() && message_id != "0002") {
    return -1;
  }

  LOG_INFO(L"Message: %s", ANSIToUnicode(message).c_str());

  memcpy(frame->data, response_frame.c_str(), response_frame.size());
  frame->datalen = response_frame.size();

  return 0;
}

int SaveRawFile(HANDLE out, unsigned char *buf, UINT len)
{
  unsigned char buf2[1500];
  UINT offset = 0;

  buf2[offset] = 0xFF;
  offset++;

  string timestamp(DateTimeFormatter::format(LocalDateTime().timestamp(), DateTimeFormat::SORTABLE_FORMAT));
  memcpy(buf2 + offset, timestamp.c_str(), timestamp.size());
  offset += timestamp.size();

  memcpy(buf2 + offset, &len, 2);
  offset += 2;

  memcpy(buf2 + offset, (char*)buf, len);
  offset += len;

  DWORD written;
  if(!WriteFile(out, buf2, offset, &written, NULL)) {
//			DWORD dwerr = GetLastError();
//			LOG_ERROR(L"Create RawData failed, error:%s", ERRORMSG(dwerr));
      return -1;
  }

  return 0;
}
