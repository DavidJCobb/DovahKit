#pragma once
#include <QString>
#include "./log_item_context.h"
#include "./log_item_type.h"

namespace dovah::notices {
   class base_error;
   class base_warning;
}

namespace ui::types {
   //
   // An entry in the Log Window, to let the user keep track of recently displayed 
   // warnings, errors, and miscellaneous messages.
   //
   class log_item {
      public:
         log_item() {}
         log_item(
            const QString&   text,
            log_item_type    type    = log_item_type::unspecified,
            log_item_context context = log_item_context::unspecified
         );
         log_item(const dovah::notices::base_error&);
         log_item(const dovah::notices::base_warning&);

      public:
         log_item_type    type    = log_item_type::unspecified;
         log_item_context context = log_item_context::unspecified;
         QString text;
         QString file;

         bool empty() const noexcept;
   };
}
