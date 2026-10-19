#pragma once
#include <cstdint>
#include "lu/bytestreams/customization/serialized_bytecount.h"
#include "lu/integral_character_code.h"
#include "./record_flags.h"

namespace dovah::tes {
   struct record_header {
      static constexpr record_flags non_data_flags = record_flags::from<record_flag::compressed>(); // when serializing a form, the serializer should decide whether these flags remain set
      
      public:
         lu::four_cc  signature = 0;
         uint32_t     size      = 0;
         record_flags flags;
         uint32_t     formID    = 0;
         union {
            struct {
               uint8_t vc_day;
               uint8_t vc_month;
               uint8_t vc_last_editor;
               uint8_t vc_current_editor;
            };
            uint32_t version_control = 0;
         };
         uint16_t version = 0;
         uint16_t version_control_2 = 0;
      
      public:
         constexpr bool body_is_compressed() const noexcept { return this->flags.has(record_flag::compressed); }
   };
   static_assert(sizeof(record_header) == 0x18);
}

namespace lu::bytestreams {
   template<typename Stream>
   struct serialized_bytecount<Stream, dovah::tes::record_header> {
      static constexpr const size_t value =  0x18;
   };
}