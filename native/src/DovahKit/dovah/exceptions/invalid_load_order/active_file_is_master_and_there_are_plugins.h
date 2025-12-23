#pragma once
#include <stdexcept>
#include "../invalid_load_order.h"

namespace dovah::exceptions::invalid_load_order_exceptions {
   class active_file_is_master_and_there_are_plugins : public invalid_load_order {
      public:
         active_file_is_master_and_there_are_plugins() : invalid_load_order("This desired active file is a master, but the load order contains plug-ins, so the active file can't be the last entry in the load order.") {}
   };
}