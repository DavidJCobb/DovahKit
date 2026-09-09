#pragma once
#include "../../../base_form_load_warning.h"
#include <limits>

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::navmesh {
   class too_many_door_links final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;

         static constexpr size_t max_supported_size = std::numeric_limits<uint16_t>::max();

      public:
         constexpr too_many_door_links(
            form_stub& subject,
            size_t     size
         )
         :
            base_form_load_warning(subject),
            size(size)
         {}

         size_t size;
   };
}
#include "../../../_util.undef.h"