#pragma once
#include "./_base.h"
#include "../_base_macros.define.h"
#include "dovah/data/story_manager.h"

namespace dovah::loaded_forms {
   class Quest;
}

namespace dovahkit::subsystems::form_info_cache::cached_data::by_form {
   class quest : public _base {
      public:
         static constexpr const auto form_types_of_interest = std::array{ dovah::form_type::quest };
         static constexpr const auto subrecords_of_interest = std::array{ 'ENAM' };
         MAKE_FORM_INFO_CACHE_DATA_TEMPLATES;

      public:
         constexpr quest() {}

         dovah::story_event_code_t event_signature = 0;

      public:
         void skim_subrecord(dovah::tes_file_reading::subrecord&);

         // Returns true if anything has changed.
         bool update(const dovah::loaded_forms::Quest&);
   };
}

#include "../_base_macros.undef.h"