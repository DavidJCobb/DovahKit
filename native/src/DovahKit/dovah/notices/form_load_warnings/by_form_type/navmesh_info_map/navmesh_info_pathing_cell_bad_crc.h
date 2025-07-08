#pragma once
#include "./_generic_crc_mismatch_notice.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::navmesh_info_map {
   class navmesh_info_pathing_cell_bad_crc final : public _generic_crc_mismatch_notice {
      public:
         MAKE_CLONE_OVERLOAD;

      public:
         constexpr navmesh_info_pathing_cell_bad_crc(
            form_stub&       subject,
            form_stub*       navmesh,
            uint32_t         crc_seen,
            uint32_t         crc_expected,
            std::string_view crc_expected_is_of
         )
         :
            _generic_crc_mismatch_notice(subject, crc_seen, crc_expected, crc_expected_is_of),
            navmesh(navmesh)
         {}

         form_stub* navmesh;
   };
}
#include "../../../_util.undef.h"