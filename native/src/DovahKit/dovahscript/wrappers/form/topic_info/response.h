#pragma once
#include <optional>
#include <string_view>
#include "../topic_info.h"
#include "dovah/data/dialogue/emotion.h"
#include "dovah/forms/TopicInfo.h"

namespace dovahscript::wrappers {
   struct topic_info_response : public wrapper_metatable {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.topic_info_response";
      static constexpr const char*   class_name      = "topic_info_response";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      using wrapped_type = dovah::loaded_forms::TopicInfo::response;

      static constexpr bool has_extra_class_setup = true;
      static void extra_class_setup(lua_State* L);

      static std::optional<dovah::dialogue::emotion> emotion_from_string(std::string_view);

      // Returns empty if no problems.
      // Else returns a suitable error string.
      static std::string verify_table_is_response_like(lua_State*, int stack_pos);

      // These do not validate the table you pass in, nor its fields, beyond nil-checking 
      // them and either leaving the destination field unchanged ("assign") or defaulting 
      // it ("overwrite").
      static void modify_from_table(
         dovah::loaded_forms::Form& dst_form,
         wrapped_type& dst,
         lua_State* L,
         int table_pos,
         bool is_overwrite
      );
   };
}