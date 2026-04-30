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

      // Collections that contain this sub-object can use this to check whether a value 
      // is valid, for a statement like `collection[n] = {}`.
      static void verify_table_can_overwrite(lua_State*, int stack_pos, const dovah::loaded_forms::Form& dst_form, const wrapped_type& dst_subobject);
      static void verify_table_for_insertion(lua_State*, int stack_pos, const dovah::loaded_forms::Form& dst_form);

      // This won't validate the table you pass in, nor its fields, beyond nil-checking 
      // them as needed to default destination fields.
      static void overwrite_with_table(
         dovah::loaded_forms::Form& dst_form,
         wrapped_type& dst,
         lua_State* L,
         int table_pos
      );
   };
}