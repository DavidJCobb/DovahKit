#pragma once
#include <optional>
#include "./_base.h"
#include "../_base_macros.define.h"
#include "dovah/data/headparts.h"
#include "dovah/data/sex.h"

namespace dovah::loaded_forms {
   class HeadPart;
}

namespace dovahkit::subsystems::form_info_cache::cached_data::by_form {
   class head_part : public _base {
      public:
         static constexpr const auto form_types_of_interest = std::array{ dovah::form_type::head_part };
         static constexpr const auto form_types_we_refer_to = std::array{ dovah::form_type::formlist };
         static constexpr const auto subrecords_of_interest = std::array{ 'DATA', 'PNAM', 'RNAM' };
         MAKE_FORM_INFO_CACHE_DATA_TEMPLATES;

      public:
         using head_part_type = dovah::head_part_type;

      public:
         dovah::form_stub* race_list = nullptr;
         //
         std::optional<dovah::sex> sex; // none == any
         //
         head_part_type type = head_part_type::misc;
         //
         bool is_extra    : 1 = false;
         bool is_playable : 1 = false;

      public:
         constexpr head_part() {}
         void skim_subrecord(dovah::tes_file_reading::subrecord&);

         // Returns true if anything has changed.
         bool sever_outbound_references_to(const dovah::form_stub*);

         // Returns true if anything has changed.
         bool update(const dovah::loaded_forms::HeadPart&);

         constexpr bool operator==(const head_part&) const noexcept = default;
   };
}


#include "../_base_macros.undef.h"