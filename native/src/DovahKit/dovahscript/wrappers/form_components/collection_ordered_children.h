#pragma once
#include <string_view>
#include "helpers/eight_cc.h"
namespace dovahscript {
   class wrapper;
}
struct lua_State;

namespace dovahscript::wrapper_likes::native_lists {
   struct form_ordered_children {
      form_ordered_children() = delete;

      public:
         static constexpr const std::string_view class_name    = "native_list<topic_info>";
         static constexpr const std::string_view metatable_key = "native_list<dovah.classes.form.ordered_children>"; 
         static constexpr const cobb::eight_cc   signature     = "OrdrChdn";

      public:
         static void define_metatable(lua_State*);
         static int push(lua_State*, const wrapper& parent);
   };
}