#pragma once
#include <cstdint>
#include <optional>
#include "./_base.h"
#include "../_base_macros.define.h"
#include "dovah/data/magic_spell_type.h"

namespace dovah::loaded_forms {
   class Enchantment;
}

namespace dovahkit::subsystems::form_info_cache::cached_data::by_form {
   class enchantment : public _base {
      public:
         static constexpr const auto form_types_of_interest = std::array{ dovah::form_type::enchantment };
         static constexpr const auto subrecords_of_interest = std::array{ 'ENIT' };
         MAKE_FORM_INFO_CACHE_DATA_TEMPLATES;

      public:
         dovah::magic_spell_type enchantment_type = dovah::magic_spell_type::enchantment_normal;

      public:
         constexpr enchantment() {}
         void skim_subrecord(dovah::tes_file_reading::subrecord&);

         // Returns true if anything has changed.
         bool update(const dovah::loaded_forms::Enchantment&);
   };
}


#include "../_base_macros.undef.h"