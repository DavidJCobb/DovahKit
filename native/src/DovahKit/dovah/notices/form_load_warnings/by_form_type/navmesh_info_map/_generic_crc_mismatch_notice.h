#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::navmesh_info_map {
   class _generic_crc_mismatch_notice : public base_form_load_warning {
      public:
         constexpr _generic_crc_mismatch_notice(
            form_stub&       subject,
            uint32_t         crc_seen,
            uint32_t         crc_expected,
            std::string_view crc_expected_is_of
         )
         :
            base_form_load_warning(subject),
            crc_seen(crc_seen),
            crc_expected(crc_expected),
            crc_expected_is_of(crc_expected_is_of)
         {}

         uint32_t    crc_seen;
         uint32_t    crc_expected;
         std::string crc_expected_is_of;
   };
}
#include "../../../_util.undef.h"