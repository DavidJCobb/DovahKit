#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_component::navmesh_pathing_cell {
   class bad_crc : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr bad_crc(
            form_stub& subject,
            form_stub* navmesh,
            uint32_t         crc_seen,
            uint32_t         crc_expected,
            std::string_view crc_expected_is_of
         )
         :
            base_form_load_warning(subject),
            crc_seen(crc_seen),
            crc_expected(crc_expected),
            crc_expected_is_of(crc_expected_is_of),
            navmesh(navmesh)
         {}

         uint32_t    crc_seen;
         uint32_t    crc_expected;
         std::string crc_expected_is_of;
         form_stub*  navmesh;
   };
}
#include "../../../_util.undef.h"