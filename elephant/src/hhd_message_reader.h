#ifndef HHD_MESSAGE_READER_H
#define HHD_MESSAGE_READER_H

#include "lock_message.h"

#define MAX_FRAME_LEN 1500

class HHDMessageReader
{
public:
  HHDMessageReader(unsigned char* buf, uint16_t len);

  bool parse(LockMessage* message);

private:
  bool check_frame();

  uint16_t unescape();

  bool parse_register_message(LockMessage* message);

  bool parse_unregister_message(LockMessage* message);

  bool parse_auth_message(LockMessage* message);

  bool parse_report_message(LockMessage* message);

  bool parse_heartbeat_message(LockMessage* message);

private:
  unsigned char   frame_[MAX_FRAME_LEN];
  unsigned char*  body_;
  uint16_t        frame_len_;
  uint16_t        body_len_;
};

#endif
