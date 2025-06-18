#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::static_collection {
   class orphaned_instances : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr orphaned_instances(
            form_stub& stub,
            size_t count
         )
         :
            base_form_load_warning(stub),
            count(count)
         {}

         size_t count;
   };
}
#include "../../../_util.undef.h"