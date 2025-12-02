#pragma once
#include "../../utils/subrecord_data_position.h"

namespace dovah::datastores::impl::idles {
   struct action_root_candidacy : public utils::subrecord_data_position {
      bool operator<(const action_root_candidacy&) const noexcept;
      constexpr bool operator==(const action_root_candidacy&) const noexcept = default;
   };

   using anam_subrecord_position = action_root_candidacy;
}