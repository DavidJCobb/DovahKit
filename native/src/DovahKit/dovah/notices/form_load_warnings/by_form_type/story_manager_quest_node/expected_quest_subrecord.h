#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::story_manager_quest_node {
   class expected_quest_subrecord final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;

      public:
         constexpr expected_quest_subrecord(
            form_stub& subject,
            size_t which,
            uint32_t signature
         )
         :
            base_form_load_warning(subject),
            which(which),
            signature(signature)
         {}

         size_t   which; // the N-th subrecord that we expected to be NNAM
         uint32_t signature;
   };
}
#include "../../../_util.undef.h"