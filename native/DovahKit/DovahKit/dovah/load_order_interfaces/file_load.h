#pragma once
#include "../callback_sets/data_load.h"
namespace dovah {
   namespace notices {
      class base_file_load_warning;
   }
   class active_load_order;
}

namespace dovah::load_order_interfaces {
   class file_load {
      friend class active_load_order;
      public:
         active_load_order& owner;
      protected:
         callback_sets::data_load callbacks;

      public:
         void log_warning(const notices::base_file_load_warning&);

      protected:
         file_load(active_load_order& o, const callback_sets::data_load& callbacks) : owner(o), callbacks(callbacks) {}
   };
}