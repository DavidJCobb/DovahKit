#include "./config.h"
#include "editor/ini/main.h"

namespace dovahkit::subsystems::worldinput::config {
   extern double press_to_long_press_threshold() {
      return dovahkit::ini::main::worldinput::fPressToLongPressThreshold.get_current_value<double>();
   }

   extern double press_to_hold_threshold() {
      return dovahkit::ini::main::worldinput::fPressToHoldThreshold.get_current_value<double>();
   }

   extern double key_sequence_expire_time() {
      return dovahkit::ini::main::worldinput::fKeySequenceExpireTime.get_current_value<double>();
   }
}