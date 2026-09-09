#pragma once
#include "../../../base_form_load_warning.h"
#include "../../../../data/dialogue/category.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::dialogue_branch {
   class invalid_topic_category : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr invalid_topic_category(
            form_stub& subject,
            dialogue::category category
         )
         :
            base_form_load_warning(subject),
            category(category)
         {}

         dialogue::category category;
   };
}
#include "../../../_util.undef.h"