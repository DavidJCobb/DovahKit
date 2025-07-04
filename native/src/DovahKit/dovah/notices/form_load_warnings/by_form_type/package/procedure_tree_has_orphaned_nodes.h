#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::package {
   class procedure_tree_has_orphaned_nodes final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;

      public:
         constexpr procedure_tree_has_orphaned_nodes(
            form_stub& subject,
            size_t orphan_count
         )
         :
            base_form_load_warning(subject),
            orphan_count(orphan_count)
         {}

         size_t orphan_count; // only counts "direct" orphans, not descendants of orphans
   };
}
#include "../../../_util.undef.h"