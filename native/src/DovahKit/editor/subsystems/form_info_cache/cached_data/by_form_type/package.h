#pragma once
#include "./_base.h"
#include "../_base_macros.define.h"
#include "dovah/data/packages/legacy_type.h"

namespace dovah::loaded_forms {
   class Package;
}

namespace dovahkit::subsystems::form_info_cache::cached_data::by_form {
   class package : public _base {
      public:
         static constexpr const auto form_types_of_interest = std::array{ dovah::form_type::package };
         static constexpr const bool read_entire_record     = true;
         MAKE_FORM_INFO_CACHE_DATA_TEMPLATES;

      public:
         constexpr package() {}

         dovah::packages::legacy_type legacy_type = dovah::packages::legacy_type::invalid;

      public:
         void skim_record(dovah::tes_file_reading::record&);

         // Returns true if anything has changed.
         bool update(const dovah::loaded_forms::Package&);
   };
}

#include "../_base_macros.undef.h"