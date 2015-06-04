#include <glog/logging.h>
#include <Poco/ByteOrder.h>

#include "hhd_message_writer.h"

using namespace std;
using namespace Poco;

namespace {
unsigned char calculate_crc(const unsigned char* buf, uint16_t len)
{
  char res = 0;

  for (int i = 0; i < len; ++i) {
    res ^= buf[i];
  }

  return res;
}

uint16_t escape(const unsigned char* src, uint16_t len, unsigned char* dest) {
  uint16_t escaped_len = 0;

  for (int i = 0; i < len; ++i) {
    if (src[i] == 0x7e) {
      *(dest + escaped_len++) = 0x7d;
      *(dest + escaped_len++) = 0x02;
    } else if (src[i] == 0x7d) {
      *(dest + escaped_len++) = 0x7d;
      *(dest + escaped_len++) = 0x01;
    } else {
      *(dest + escaped_len++) = src[i];
    }
  }

  return escaped_len;
}

}

HHDMessageWriter::HHDMessageWriter()
  :frame_len_(0),body_len_(0)
{
  body_ = frame_ + 12;
}

uint16_t HHDMessageWriter::generate_frame(LockMessage* message, unsigned char* frame)
{
  *(uint16_t*)frame_ = ByteOrder::toNetwork(message->id_);
  memcpy(frame_ + 4, message->lock_id_, 6);
  *(uint16_t*)(frame_ + 10) = ByteOrder::toNetwork(message->sequence_num_);

  if (message->id_ == 0x8100) {
    write_register_frame(message);
  }
  else if (message->id_ == 0x8001) {
    write_general_frame(message);
  }
  else if (message->id_ == 0x8900) {
    write_platform_frame(message);
  } else {
    LOG(ERROR) << "Invalid message id: " << message->id_;
  }

  // set 808 header body length
  *(uint16_t*)(frame_ + 2) = ByteOrder::toNetwork(body_len_ & 0x01FF);

  return process_crc_and_escape(frame);
}

void HHDMessageWriter::write_register_frame(LockMessage* message)
{
  *(uint16_t*)body_ = message->sequence_num_;
  *(body_ + 2)      = message->platform_result_;
  memcpy(body_ + 3, message->code_.data(), message->code_.size());

  body_len_         = 3 + message->code_.size();
}

void HHDMessageWriter::write_general_frame(LockMessage* message)
{
  *(uint16_t*)body_       = message->sequence_num_;
  *(uint16_t*)(body_ + 2) = message->response_id_;
  *(body_ + 4)            = message->platform_result_;

  body_len_ = 5;
}

void HHDMessageWriter::write_platform_frame(LockMessage* message)
{
  *body_ = 0x33;
  if (message->console_cmd_ == CONSOLE_REPORT_SET_INTERVAL) {
    string cmd( "*S01,04," + message->string_ + "#");
    memcpy(body_ + 1, cmd.c_str(), cmd.size());
    body_len_ = cmd.size() + 1;
  }
  else if (message->console_cmd_ == CONSOLE_REPORT_SET_LOWVALTAGE) {
    string cmd( "*S03,03," + message->string_ + "#");
    memcpy(body_ + 1, cmd.c_str(), cmd.size());
    body_len_ = cmd.size() + 1;
  }
  else if (message->console_cmd_ == CONSOLE_REPORT_REBOOT) {
    string cmd("*S05,00,#");
    memcpy(body_ + 1, cmd.c_str(), cmd.size());
    body_len_ = cmd.size() + 1;
  }
  else if (message->console_cmd_ == CONSOLE_REPORT_RESTORE) {
    string cmd("*S06,00,#");
    memcpy(body_ + 1, cmd.c_str(), cmd.size());
    body_len_ = cmd.size() + 1;
  }
  else if (message->console_cmd_ == CONSOLE_REPORT_SET_ICNUM) {
    string ic_array;
    for (uint8_t i = 0; i < message->array_.size(); ++i) {
      string ic(message->array_[i]);
      if (ic == "0") {
        ic = "000000";
      }
      ic_array += ic;
    }
  }

  body_len_ = 5;
}

uint16_t HHDMessageWriter::process_crc_and_escape(unsigned char *frame)
{
  unsigned char crc = calculate_crc(frame_, 12 + body_len_);
  frame_[12 + body_len_] = crc;

  uint16_t escape_len = escape(frame_, 12 + body_len_ + 1, frame + 1);
  frame[0] = 0x7e;
  frame[escape_len + 1] = 0x7e;

  return 1 + escape_len + 1;
}

