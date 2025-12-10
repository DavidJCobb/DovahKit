#pragma once
#include "../../utils/subrecord_data_position.h"

namespace dovah::datastores::impl::camera_paths {
   struct anam_subrecord_position : public utils::subrecord_data_position {
      bool operator<(const anam_subrecord_position&) const noexcept;
      constexpr bool operator==(const anam_subrecord_position&) const noexcept = default;
   };
}