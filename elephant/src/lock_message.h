#ifndef LOCK_MESSAGE_H
#define LOCK_MESSAGE_H

#include <stdint.h>
#include <string>

#define JT808_TEID_LEN				6

enum {
  CONSOLE_REPORT = 1,
  CONSOLE_REPORT_SEAL,
  CONSOLE_REPORT_UNSEAL,
  CONSOLE_REPORT_GET_ICNUM,
  CONSOLE_REPORT_GET_SEALS,
  CONSOLE_REPORT_SET_ICNUM,
  CONSOLE_REPORT_SET_SEALS,
  CONSOLE_REPORT_SET_INTERVAL,
  CONSOLE_REPORT_GET_INTERVAL,
  CONSOLE_REPORT_SET_LOWVALTAGE,
  CONSOLE_REPORT_GET_LOWVALTAGE,
  CONSOLE_REPORT_REBOOT,
  CONSOLE_REPORT_RESTORE,
};

struct LockMessage
{
	uint16_t      id_;
  unsigned char lock_id_[6];
  uint16_t      sequence_num_;

  //GPS
  uint32_t      state_;
  uint32_t      latitude_;
  uint32_t      longitude_;
  uint16_t      altitude_;
  uint16_t      speed_;
  uint16_t      direction_;
  unsigned char timestamp_[6];

  // Lock state
  bool          lock_state_;
  bool          seal_state_;
  uint16_t      lock_valtage_;
  bool          anti_dismantle_;
  uint8_t       event_;
  std::string   operation_id_;

  // console cmd
  uint8_t       console_cmd_;
  std::vector<std::string> array_;
  std::string   string_;
  bool          result_;

  // other
  uint16_t      response_id_;

  // register
  std::string   maker_;
  std::string   module_;
  std::string   serial_id_;
  std::string   plate_;

  // platform response result
  unsigned char platform_result_;

  // auth
  std::string   code_;
};
#endif
