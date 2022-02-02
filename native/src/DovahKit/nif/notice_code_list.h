#pragma once
#include "notice_code_t.h"

namespace nifDK {
   struct notice_code {
      notice_code()       = delete;
      enum type : notice_code_t {
         none = 0x00000000,
         //
         stream_ended_early         = 0x00000001,
         line_string_with_bad_end   = 0x00000002, // line-string ending in "\r" but not "\r\n"
         string_index_out_of_bounds = 0x00000003,
      };
   };
}
