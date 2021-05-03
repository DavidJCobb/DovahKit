#pragma once
#include "../topic_info.h"

#include "../../../../dovah/forms/TopicInfo.h"

namespace editor_script::wrappers {
   struct topic_info_response : public wrapper_metatable {
      static constexpr const char* superclass_key = metatable_key;
      static constexpr const char* metatable_key  = "dovah.classes.topic_info_response";
      static constexpr const char* class_name     = "topic_info_response";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;

      using wrapped_t = dovah::loaded_forms::TopicInfo::response;
      static wrapped_t* unwrap(wrapper& w);
   };
}