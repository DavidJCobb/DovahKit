#pragma once
#include "./_base.h"
#include "../_base_macros.define.h"

namespace dovah::loaded_forms {
   class Faction;
}

namespace dovahkit::subsystems::form_info_cache::cached_data::by_form {
   class faction : public _base {
      public:
         static constexpr const auto form_types_of_interest = std::array{ dovah::form_type::faction };
         static constexpr const auto subrecords_of_interest = std::array{ 'DATA' };
         MAKE_FORM_INFO_CACHE_DATA_TEMPLATES;

      public:
         constexpr faction() {}

         bool tracks_crime = false;

      public:
         void skim_subrecord(dovah::tes_file_reading::subrecord&);

         // Returns true if anything has changed.
         bool update(const dovah::loaded_forms::Faction&);
   };
}

#include "../_base_macros.undef.h"