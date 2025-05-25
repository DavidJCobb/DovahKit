#pragma once
#include "dovah/form_types.h"

namespace dovah {
   namespace tes_file_reading {
      class subrecord;
   }
   class form_reference_t;
   class form_stub;
}

namespace dovahkit::subsystems::form_info_cache::cached_data::by_form {
   class _base {
      public:
         #pragma region Members to override
            static constexpr const std::array<dovah::form_type, 0> form_types_of_interest = {};
            static constexpr const std::array<uint32_t, 0>         subrecords_of_interest = {};

            // If the cached info includes a form stub pointer, then list the expected type of 
            // the pointed-to form in your override of this member.
            static constexpr const std::array<dovah::form_type, 0> form_types_we_refer_to = {};
         #pragma endregion

      public:
         constexpr _base() {}

         constexpr bool operator==(const _base&) const noexcept = default;

      protected:
         void _read_form_stub_from_subrecord(dovah::form_stub*& dst, dovah::tes_file_reading::subrecord&, dovah::form_type allowed_type);
         void _update_form_stub_from_loaded(dovah::form_stub*& dst, const dovah::form_reference_t& src, dovah::form_type allowed_type);
   };
}