#include "response.h"
#include <array>
#include <string_view>
#include <utility> // std::pair
#include "helpers/lua/error.h"
#include "dovahscript/push_native_object.h"
#include "dovahscript/wrapper.h"
#include "./collection_responses.h"

// For table-to-response ops:
#include "dovahscript/api_helpers/subobject_property_helpers/common_lambdas.h"
#include "dovahscript/api_helpers/subobject_property_helpers/make_getters_and_setters.h"
#include "dovahscript/api_helpers/subobject_property_helpers/property_definition.h"
#include "dovahscript/api_helpers/subobject_property_helpers/types/enumeration.h"
#include "dovahscript/api_helpers/subobject_property_helpers/types/form.h"
#include "dovahscript/api_helpers/subobject_property_helpers/types/integer.h"
#include "dovahscript/api_helpers/subobject_property_helpers/types/localized_string.h"
#include "dovahscript/api_helpers/subobject_property_helpers/types/std_string.h"
//
#include "dovahscript/api_helpers/subobject_property_helpers/apply_table_variations.h"
#include "dovahscript/api_helpers/subobject_property_helpers/verify_table_for_lua_assignment.h"

#include "helpers/attributes/forceinline.h"
#define SUBOBJECT_FIELD_ACCESSOR(prop) [](wrapped_type& src) COBB__FORCEINLINE -> auto& { return src.prop; }

namespace {
   using namespace dovahscript;
   using cls          = wrappers::topic_info_response;
   using form_type    = dovah::loaded_forms::TopicInfo;
   using wrapped_type = cls::wrapped_type;

   static wrapped_type* _unwrap(wrapper& w) {
      if (w.is_collection)
         return nullptr;
      if (w.parts[0].signature != wrapper_likes::native_lists::topic_info_responses::signature)
         return nullptr;
      auto* form = w.get_loaded_form_data<form_type>();
      if (!form)
         return nullptr;
      auto& list = form->responses;
      auto  i = w.parts[0].index;
      if (i >= list.size())
         return nullptr;
      return &list[i];
   }
   static wrapped_type& _unwrap_and_require(lua_State* L) {
      auto& self = get_wrapper_for_thiscall<cls>(L);
      auto* data = _unwrap(self);
      if (data == nullptr)
         cobb::lua::error(L, "topic_info_response wrapper has no underlying object (deleted?)");
      return *data;
   }

   constexpr const auto emotion_type_names = []() {
      using pair_type = std::pair<dovah::dialogue::emotion, std::string_view>;
      return std::array{
         pair_type{ dovah::dialogue::emotion::anger,    "anger" },
         pair_type{ dovah::dialogue::emotion::disgust,  "disgust" },
         pair_type{ dovah::dialogue::emotion::fear,     "fear" },
         pair_type{ dovah::dialogue::emotion::happy,    "happy" },
         pair_type{ dovah::dialogue::emotion::neutral,  "neutral" },
         pair_type{ dovah::dialogue::emotion::puzzled,  "puzzled" },
         pair_type{ dovah::dialogue::emotion::sad,      "sad" },
         pair_type{ dovah::dialogue::emotion::surprise, "surprise" },
      };
   }();
}

namespace subobject_property_helpers {
   using namespace api_helpers::subobject_property_helpers;
}

constexpr auto subobject_properties = std::tuple{
   subobject_property_helpers::std_string_property::define(
      "edits",
      SUBOBJECT_FIELD_ACCESSOR(edits)
   ),

   subobject_property_helpers::enumeration_property<emotion_type_names>::define(
      "emotion_type",
      SUBOBJECT_FIELD_ACCESSOR(emotion.type),
      dovah::dialogue::emotion::neutral
   ),
   
   subobject_property_helpers::integer_property<0, 100>::define(
      "emotion_value",
      SUBOBJECT_FIELD_ACCESSOR(emotion.value),
      50
   ),
   
   subobject_property_helpers::form_property<dovah::form_type::idle>::define(
      "listener_idle",
      SUBOBJECT_FIELD_ACCESSOR(idles.listener)
   ),
   
   subobject_property_helpers::std_string_property::define(
      "script_notes",
      SUBOBJECT_FIELD_ACCESSOR(script_notes)
   ),

   subobject_property_helpers::form_property<dovah::form_type::idle>::define(
      "speaker_idle",
      SUBOBJECT_FIELD_ACCESSOR(idles.speaker)
   ),

   subobject_property_helpers::form_property<dovah::form_type::sound_descriptor>::define(
      "substitute_sound",
      SUBOBJECT_FIELD_ACCESSOR(sound)
   ),

   subobject_property_helpers::localized_string_property::define(
      "text",
      SUBOBJECT_FIELD_ACCESSOR(text)
   ),

   subobject_property_helpers::property_definition{
      .name   = "unique_id",
      .access = SUBOBJECT_FIELD_ACCESSOR(id),
      .check  = [](lua_State* L, int pos) -> std::string_view {
         if (!lua_isinteger(L, pos))
            return "integer expected";
         auto v = lua_tointeger(L, pos);
         if (v < 0 || v > form_type::max_available_response_ids)
            return "values must be in the range [0, 255], though using 0 is discouraged";
         if (v == 0)
            lua_warning(L, "a response ID of 0 is invalid (it is the sentinel used while recording lines in the Creation Kit)", 0);
         return {};
      },
      .pull = &subobject_property_helpers::pull::integer,
      .push = [](lua_State* L, const decltype(wrapped_type::id)& value) { lua_pushinteger(L, value); },

      // Ensure uniqueness.
      .late_check = [](lua_State* L, const dovah::loaded_forms::Form& form, const wrapped_type* subobject, const lua_Integer& v) -> void {
         if (subobject && subobject->id == v)
            return;
         assert(form.stub.form_type == dovah::form_type::topic_info);
         const auto& info = static_cast<const dovah::loaded_forms::TopicInfo&>(form);
         if (subobject) {
            for (const auto& resp : info.responses)
               if (resp.id == v && &resp != subobject)
                  luaL_error(L, "the provided response ID (%u) is already in use by another response on this TopicInfo", v);
         } else {
            for (const auto& resp : info.responses)
               if (resp.id == v)
                  luaL_error(L, "the provided response ID (%u) is already in use by another response on this TopicInfo", v);
         }
      },

      .treat_nil_as_unchanged = true,
   },
};

/*static*/ void cls::verify_table_can_overwrite(lua_State* L, int pos, const dovah::loaded_forms::Form& dst_form, const wrapped_type& dst_subobject) {
   return subobject_property_helpers::verify_table_for_lua_assignment<wrapped_type, subobject_properties>(L, pos, dst_form, dst_subobject);
}
/*static*/ void cls::verify_table_for_insertion(lua_State* L, int pos, const dovah::loaded_forms::Form& dst_form) {
   return subobject_property_helpers::verify_table_for_lua_insertion<wrapped_type, subobject_properties>(L, pos, dst_form);
}
/*static*/ void cls::overwrite_with_table(
   dovah::loaded_forms::Form& dst_form,
   wrapped_type& dst_data,
   lua_State* L,
   int  table_pos
) {
   subobject_property_helpers::apply_table<wrapped_type, true, subobject_properties>(L, table_pos, dst_form, dst_data);
}

namespace {
   namespace _methods {
   }
   namespace _getters {
      int parent(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* data = _unwrap(self);
         if (data == nullptr)
            cobb::lua::error(L, "topic_info_response wrapper has no underlying object (deleted?)");
         return push_native_object(self.stub);
      }
   }
   namespace _setters {
   }
}

namespace dovahscript::wrappers {
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_methods = {
      { "assign",         subobject_property_helpers::assign_table_via_lua_method<cls, _unwrap_and_require, subobject_properties> },
      { "overwrite_with", subobject_property_helpers::overwrite_with_table_via_lua_method<cls, _unwrap_and_require, subobject_properties> },
   };
   
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_getters = {
      { "parent", &_getters::parent }, // get containing info
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = no_functions;

   /*static*/ void cls::extra_class_setup(lua_State* L) {
      subobject_property_helpers::setup_extra_getters_and_setters<cls, &_unwrap_and_require, subobject_properties>(L);
   }
}