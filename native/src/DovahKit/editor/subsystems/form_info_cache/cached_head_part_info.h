#pragma once
#include <optional>
#include "dovah/data/sex.h"
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

      using head_part_type = dovah::head_part_type;

      dovah::form_stub* race_list = nullptr;
      //
      std::optional<dovah::sex> sex; // none == any
      //
      head_part_type type = head_part_type::misc;
      //
      bool is_extra    : 1 = false;
      bool is_playable : 1 = false;

      void skim_subrecord(dovah::tes_file_reading::subrecord&);

      // Returns true if anything has changed.
      bool sever_outbound_references_to(const dovah::form_stub*);

      // Returns true if anything has changed.
      bool update(const dovah::loaded_forms::HeadPart&);

      constexpr bool operator==(const cached_head_part_info&) const noexcept = default;
   };
}