#pragma once
#include "form.h"

#include "../../../dovah/forms/TopicInfo.h"

namespace editor_script::wrapper_part_types {
   inline constexpr cobb::eight_cc topic_info_response = "InfoResp";
}

namespace editor_script::wrappers {
   struct topic_info : public form {
      static constexpr const char* superclass_key = metatable_key;
      static constexpr const char* metatable_key  = "dovah.classes.topic_info";
      static constexpr const char* class_name     = "topic_info";
      static std::initializer_list<luaL_Reg> metatable_methods;
      static std::initializer_list<luaL_Reg> metatable_getters;
      static std::initializer_list<luaL_Reg> metatable_setters;

      static constexpr const char* response_collection_key = "collection<dovah.classes.topic_info.responses>";
      static void build_collection_metatables(lua_State* L);
   };
}