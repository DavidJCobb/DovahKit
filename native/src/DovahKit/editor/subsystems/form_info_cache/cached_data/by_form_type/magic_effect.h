#pragma once
#include <cstdint>
#include <optional>
#include "./_base.h"
#include "../_base_macros.define.h"
#include "dovah/data/magic_casting_type.h"
#include "dovah/data/magic_delivery_type.h"

namespace dovah::loaded_forms {
   class MagicEffect;
}

namespace dovahkit::subsystems::form_info_cache::cached_data::by_form {
   class magic_effect : public _base {
      public:
         static constexpr const auto form_types_of_interest = std::array{ dovah::form_type::magic_effect };
         static constexpr const auto subrecords_of_interest = std::array{ 'DATA' };
         MAKE_FORM_INFO_CACHE_DATA_TEMPLATES;

      public:
         dovah::magic_casting_type  casting_type  = dovah::magic_casting_type::concentration;
         dovah::magic_delivery_type delivery_type = dovah::magic_delivery_type::self;
         int32_t magic_school = -1;

      public:
         constexpr magic_effect() {}
         void skim_subrecord(dovah::tes_file_reading::subrecord&);

         // Returns true if anything has changed.
         bool update(const dovah::loaded_forms::MagicEffect&);
   };
}


#include "../_base_macros.undef.h"