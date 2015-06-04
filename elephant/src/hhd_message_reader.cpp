#include <stdint.h>
#include <string>
#include <vector>
#include <Poco/ByteOrder.h>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <glog/logging.h>

#include "hhd_message_reader.h"

using namespace std;
using namespace Poco;

namespace {
uint16_t unescape(unsigned char* src, uint16_t len, unsigned char* dest)
{
  bool last_element_escape = false;
  uint16_t escape_number = 0;

  for (int i = 0; i < len - 1; ++i) {
    if (src[i] == 0x7d && (src[i+1] == 0x02 || src[i+1] == 0x01)) {
      if(src[i+1] == 0x02) {
        *dest++ = 0x7e;
      } else if (src[i+1] == 0x01) {
        *dest++ = 0x7d;
      }

      ++escape_number;
      ++i;
      if (i == (len - 1)) {
        last_element_escape = true;
      }
    } else {
      *dest++ = src[i];
    }
  }

  if (!last_element_escape) {
    *dest = src[len - 1];
  }

  return len - escape_number;
}

unsigned char to_hex(unsigned char dec)
{
  return (dec / 10) * 0x10 + dec % 10;
}
unsigned char calculate_crc(const char* buf, uint16_t len)
{
  unsigned char res = 0;

  for (int i = 0; i < len; ++i) {
    res ^= buf[i];
  }

  return res;
}

std::string to_str(const unsigned char byte)
{
  char buf[2];
  char tmp  = ((byte & 0xF0) >> 4);
  if (tmp > 10) {
    buf[0] = tmp - 10 + 'A';
  } else {
    buf[0] = tmp + '0';
  }
  tmp = byte & 0x0F;
  if (tmp > 10) {
    buf[1] = tmp - 10 + 'A';
  } else {
    buf[1] = tmp + '0';
  }
  std::string res(buf, 2);
  return res;
}
} /// namespace

HHDMessageReader::HHDMessageReader(unsigned char* frame, uint16_t len)
  :frame_len_(len)
{
  memcpy(frame_, frame, frame_len_);
}

bool HHDMessageReader::check_frame()
{
  if (frame_len_ < 12) {
    LOG(INFO) << "frame is too short: " << frame_len_;
    return false;
  }

  if ((frame_[0] != 0x7e) || (frame_[frame_len_-1] != 0x7e)) {
    LOG(INFO) << "Invalid frame, frame not start or end with 0x7e";
    return false;
  }

  frame_len_ = unescape();

  unsigned char crc = calculate_crc((char*)frame_, frame_len_ - 1);
  if (crc != frame_[frame_len_- 1]) {
    LOG(INFO) << "Frame crc check failed, drop it";
    for (int i = 0; i < frame_len_ -1; ++i) {
      printf("%x ", frame_[i]);
    }
    printf("\n=============== %x \n", crc);
    return false;
  }
  frame_len_--;

  return true;
}

uint16_t HHDMessageReader::unescape()
{
  return ::unescape(frame_ + 1, frame_len_ - 2, frame_);
}

bool HHDMessageReader::parse(LockMessage* message)
{
  if (!check_frame()) {
    return false;
  }

  body_ = frame_ + 12;
  message->id_ = ByteOrder::fromNetwork(*(uint16_t*)frame_);
  body_len_ = ByteOrder::fromNetwork(*(uint16_t*)(frame_ + 2)) & 0x01FF;
  memcpy(message->lock_id_, frame_ + 4, 6);
  message->sequence_num_ = ByteOrder::fromNetwork(*(uint16_t*)(frame_ + 10));

  if (message->id_ == 0x0100) {
    return parse_register_message(message);
  }
  else if (message->id_ == 0x0003) {
    return parse_unregister_message(message);
  }
  else if (message->id_ == 0x0102) {
    return parse_auth_message(message);
  }
  else if (message->id_ == 0x0200) {
    return parse_report_message(message);
  }
  else if (message->id_ == 0x0002) {
    return parse_heartbeat_message(message);
  }
  else {
    LOG(INFO) << "Unknown Message id, dropping frame";
  }

  return false;
}

bool HHDMessageReader::parse_register_message(LockMessage* message)
{
  //TODO: 从锁的消息里面获取制造商，型号，序列号等信息
  message->maker_     = "HHD";
  message->module_    = "0000";
  message->serial_id_ = "0000";
  message->plate_     = "0000";

  return true;
}

bool HHDMessageReader::parse_unregister_message(LockMessage* message)
{
  // nothing to do
  return true;
}

bool HHDMessageReader::parse_auth_message(LockMessage* message)
{
  message->code_ = string((char*)body_, body_len_);
  return true;
}

bool HHDMessageReader::parse_report_message(LockMessage* message)
{
  //GPS
  message->state_     = ByteOrder::fromNetwork(*(uint32_t*)(body_ + 4));
  message->latitude_  = ByteOrder::fromNetwork(*(uint32_t*)(body_ + 8));
  message->longitude_ = ByteOrder::fromNetwork(*(uint32_t*)(body_ + 12));
  message->altitude_  = ByteOrder::fromNetwork(*(uint16_t*)(body_ + 16));
  message->speed_     = ByteOrder::fromNetwork(*(uint16_t*)(body_ + 18)) * 10;
  message->direction_ = ByteOrder::fromNetwork(*(uint16_t*)(body_ + 20));
  memcpy(message->timestamp_, body_ + 22, 6);

  //TODO extra info
  if (*(body_ + 34) != 0x33) {
    LOG(INFO) << "Invalid position report request, cannot find 0x33 message id";
    return false;
  }

  unsigned char* extra_body = body_ + 36;
  uint8_t extra_body_len = *(body_ + 35);

  if (*extra_body != '*' || *(extra_body + extra_body_len -1) != '#') {
    LOG(INFO) << "Invalid position report request, invalid frame begin or end";
    return false;
  }

  vector<string> vec;
  string tmp(extra_body + 1, extra_body + extra_body_len - 1);
  boost::split(vec, tmp, boost::is_any_of(","));

  if (vec.size() != 3 && vec.size() != 4) {
    LOG(INFO) << "Invalid extra cmd format, dropping: " << vec.size();
    return  false;
  }

  if (vec[0] == "M00") {
    message->console_cmd_ = CONSOLE_REPORT;

    const char* ptr = vec[2].data();
    message->lock_state_ = (*ptr++ == '0' ? 0 : 1);
    message->seal_state_ = (*ptr++ == '0' ? 0 : 1);
    message->lock_valtage_ = boost::lexical_cast<uint16_t>(string(ptr, 3)) * 10;
    ptr += 3;
    message->anti_dismantle_ = (*ptr++ == '0' ? 0 : 1);
    message->event_ = *ptr++;
    message->operation_id_ = string(ptr, vec[2].size() - 8);
  }
  else if (vec[0] == "M01") {
    message->console_cmd_ = CONSOLE_REPORT_SEAL;
    message->result_      = (vec[2] == "0" ? false : true);
  }
  else if (vec[0] == "M02") {
    message->console_cmd_ = CONSOLE_REPORT_UNSEAL;
    message->result_      = (vec[2] == "0" ? false : true);
  }
  else if (vec[0] == "M06") {
    message->console_cmd_ = CONSOLE_REPORT_SET_ICNUM;
    message->result_      = (vec[2] == "0" ? false : true);
  }
  else if (vec[0] == "M07") {
    message->console_cmd_ = CONSOLE_REPORT_GET_ICNUM;
    for (uint8_t i = 0; i < vec[3].size(); i += 8) {
      message->array_.push_back(string(vec[3].data() + i, 8));
    }
  }
  else if (vec[0] == "M08") {
    message->console_cmd_ = CONSOLE_REPORT_SET_SEALS;
    message->result_      = (vec[2] == "0" ? false : true);
  }
  else if (vec[0] == "M09") {
    message->console_cmd_ = CONSOLE_REPORT_GET_SEALS;
    for (uint8_t i = 0; i < vec[3].size(); i += 6) {
      message->array_.push_back(string(vec[3].data() + i, 6));
    }
  }
  else if (vec[0] == "S01") {
    message->console_cmd_ = CONSOLE_REPORT_SET_INTERVAL;
    message->result_      = (vec[2] == "0" ? false : true);
  }
  else if (vec[0] == "S02") {
    message->console_cmd_ = CONSOLE_REPORT_GET_INTERVAL;
    message->string_ = vec[2];
  }
  else if (vec[0] == "S03") {
    message->console_cmd_ = CONSOLE_REPORT_SET_LOWVALTAGE;
    message->result_      = (vec[2] == "0" ? false : true);
  }
  else if (vec[0] == "S04") {
    message->console_cmd_ = CONSOLE_REPORT_GET_LOWVALTAGE;
    message->string_ = vec[2];
  }
  else if (vec[0] == "S05") {
    message->console_cmd_ = CONSOLE_REPORT_REBOOT;
    message->result_      = (vec[2] == "0" ? false : true);
  }
  else if (vec[0] == "S06") {
    message->console_cmd_ = CONSOLE_REPORT_RESTORE;
    message->result_      = (vec[2] == "0" ? false : true);
  }

  return true;
}

bool HHDMessageReader::parse_heartbeat_message(LockMessage* message)
{
  // nothing to do
  return true;
}
