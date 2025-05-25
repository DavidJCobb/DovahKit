#pragma once
#include "./_base.h"
#include "../_base_macros.define.h"

namespace dovah::loaded_forms {
   class Voicetype;
}

namespace dovahkit::subsystems::form_info_cache::cached_data::by_form {
   class voicetype : public _base {
      public:
         static constexpr const auto form_types_of_interest = std::array{ dovah::form_type::voicetype };
         static constexpr const auto subrecords_of_interest = std::array{ 'DNAM' };
         MAKE_FORM_INFO_CACHE_DATA_TEMPLATES;

      public:
         constexpr voicetype() {}

         bool allow_default_dialogue = false;
         bool female = false;

      public:
         void skim_subrecord(dovah::tes_file_reading::subrecord&);

         // Returns true if anything has changed.
         bool update(const dovah::loaded_forms::Voicetype&);
   };
}

#include "../_base_macros.undef.h"