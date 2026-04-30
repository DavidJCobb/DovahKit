#pragma once
#include <string_view>
#include "helpers/eight_cc.h"
namespace dovahscript {
   class wrapper;
}
struct lua_State;

namespace dovahscript::wrapper_likes::native_lists {
   struct topic_info_responses {
      topic_info_responses() = delete;

      public:
         static constexpr const std::string_view class_name    = "native_list<topic_info_response>";
         static constexpr const std::string_view metatable_key = "native_list<dovah.classes.topic_info.responses>"; 
         static constexpr const cobb::eight_cc   signature     = "InfoResp";

      public:
         static void define_metatable(lua_State*);
         static int push(lua_State*, const wrapper& parent);
   };
}