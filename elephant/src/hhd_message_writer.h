#ifndef HHD_MESSAGE_WRITER_H
#define HHD_MESSAGE_WRITER_H

#include "lock_message.h"

class HHDMessageWriter
{
public:
  HHDMessageWriter();

  uint16_t generate_frame(LockMessage* message, unsigned char* frame);

private:
  void write_register_frame(LockMessage* message);

  void write_general_frame(LockMessage* message);

  void write_platform_frame(LockMessage* message);

  uint16_t process_crc_and_escape(unsigned char *frame);

private:
  unsigned char   frame_[1500];
  uint16_t        frame_len_;
  unsigned char*  body_;
  uint16_t        body_len_;
};
#endif
