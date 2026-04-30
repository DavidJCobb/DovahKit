#pragma once
#include "form.h"

namespace dovah::loaded_forms {
   class TopicInfo;
}

namespace dovahscript::wrappers {
   struct topic_info : public form {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.topic_info";
      static constexpr const char*   class_name      = "topic_info";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      using wrapped_type = dovah::loaded_forms::TopicInfo;

      static constexpr bool has_extra_class_setup = true;
      static void extra_class_setup(lua_State* L);
   };
}