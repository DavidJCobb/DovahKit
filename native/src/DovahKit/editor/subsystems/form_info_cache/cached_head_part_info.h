#pragma once
#include "dovah/forms/HeadPart.h"

namespace dovah {
   namespace loaded_forms {
      class HeadPart;
   }
   namespace tes_file_reading {
      class subrecord;
   }
}

namespace dovahkit::subsystems::form_info_cache {
   struct cached_head_part_info {
      constexpr cached_head_part_info() {}

      using type = dovah::loaded_forms::HeadPart::head_part_type;
      enum class sex : uint8_t {
         any,
         female,
         male,
      };

      dovah::form_stub* race_list = nullptr;
      enum sex  sex         : 2 = sex::any;
      enum type type        : 3 = type::misc;
      bool      is_extra    : 1 = false;
      bool      is_playable : 1 = false;

      void skim_subrecord(dovah::tes_file_reading::subrecord&);

      // Returns true if anything has changed.
      bool sever_outbound_references_to(const dovah::form_stub*);

      // Returns true if anything has changed.
      bool update(const dovah::loaded_forms::HeadPart&);

      constexpr const bool operator==(const cached_head_part_info&) const = default;
   };
}