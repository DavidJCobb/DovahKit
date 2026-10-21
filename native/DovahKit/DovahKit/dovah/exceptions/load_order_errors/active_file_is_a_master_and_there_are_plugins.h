#pragma once
#include "../load_order_error.h"

namespace dovah::exceptions::load_order_errors {
   class active_file_is_a_master_and_there_are_plugins : public load_order_error {
      public:
         using load_order_error::load_order_error;
   };
}