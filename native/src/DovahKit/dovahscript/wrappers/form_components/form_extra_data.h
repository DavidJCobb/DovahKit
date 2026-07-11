#pragma once
#include "helpers/eight_cc.h"
#include "../base.h"
namespace dovah::loaded_forms::components {
   class extra_data_list;
}
namespace dovahscript {
   class wrapper;
}

namespace dovahscript::wrappers {
   struct form_extra_data : public wrapper_metatable {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.form_extra_data";
      static constexpr const char*   class_name      = "form_extra_data";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      using wrapped_type = dovah::loaded_forms::components::extra_data_list;
      static constexpr cobb::eight_cc wrapper_part_type = "XtraData";

      static wrapped_type* unwrap(wrapper&);
   };
}