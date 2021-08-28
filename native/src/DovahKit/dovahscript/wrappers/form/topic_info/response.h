#pragma once
#include "../topic_info.h"

#include "../../../../dovah/forms/TopicInfo.h"

namespace dovahscript::wrappers {
   struct topic_info_response : public wrapper_metatable {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.topic_info_response";
      static constexpr const char*   class_name      = "topic_info_response";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      using wrapped_type = dovah::loaded_forms::TopicInfo::response;
      static wrapped_type* unwrap(wrapper& w);
   };
}