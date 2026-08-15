#pragma once
#include "./_base.h"
#include "../_base_macros.define.h"

namespace dovah::loaded_forms {
   class CollisionLayer;
}

namespace dovahkit::subsystems::form_info_cache::cached_data::by_form {
   class collision_layer : public _base {
      public:
         static constexpr const auto form_types_of_interest = std::array{ dovah::form_type::collision_layer };
         static constexpr const auto subrecords_of_interest = std::array{ 'BNAM', 'GNAM' };
         MAKE_FORM_INFO_CACHE_DATA_TEMPLATES;

      public:
         uint32_t unique_id = 0; // BNAM
         struct {
            bool sensor  : 1 = false;
            bool trigger : 1 = false;
         } flags; // GNAM

      public:
         constexpr collision_layer() {}
         void skim_subrecord(dovah::tes_file_reading::subrecord&);

         // Returns true if anything has changed.
         bool update(const dovah::loaded_forms::CollisionLayer&);

         constexpr bool operator==(const collision_layer&) const noexcept = default;
   };
}


#include "../_base_macros.undef.h"