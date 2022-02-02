#pragma once
#include <string>
#include <type_traits>
#include "notice_code_t.h"

namespace nifDK {
   struct detailed_notice {
      enum class notice_type {
         unspecified = 0,
         warning     = 1,
         error       = 2,
      };

      struct flag {
         flag() = delete;
         enum type {
            has_file_offset = 0x00000001,
            has_cause_block = 0x00000002,
         };
      };
      using flags_t = std::underlying_type_t<flag::type>;

      notice_type    type    = notice_type::unspecified;
      notice_code_t  code    = default_notice_code;
      flags_t        flags   = 0;
      uint32_t       offset  = 0;
      struct {
         struct {
            int32_t     index = -1;
            std::string name;
         } block;
      } cause;
   };
}