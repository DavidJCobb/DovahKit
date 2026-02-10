#include "./collection_array_property_values.h"
#include "helpers/lua/error.h"
#include "helpers/lua/warning.h"
#include "helpers/type_traits/is_std_vector.h"
#include "dovahscript/core/subsystems/permissions.h"
#include "dovahscript/core/subsystems/userdata.h"
#include "dovahscript/core/classes.h"
#include "dovahscript/pull_native_object.h"
#include "dovahscript/push_native_object.h"
#include "dovahscript/wrapper.h"

#include "dovah/forms/components/papyrus/property.h"
#include "dovah/forms/components/papyrus/property_object_value.h"
#include "dovah/forms/Quest.h"

#include "../../form/form.h"
#include "../../form/quest/alias.h"
#include "./property.h"

namespace {
   constexpr const char* collection_metatable_key = "collection<dovah.classes.papyrus.array_property_values>";
}

namespace {
   using namespace dovahscript;
   using prop_wrapped_type     = wrappers::papyrus_property::wrapped_type;
   using property_object_value = dovah::loaded_forms::components::papyrus::property_object_value;
   using property_type         = dovah::loaded_forms::components::papyrus::property_type;
   
   static wrapper& get_collection_wrapper(lua_State* L) {
      auto* self = (wrapper*) classes::cast_to_class(L, 1, collection_metatable_key);
      if (self == nullptr) {
         cobb::lua::error(L, "function called with bad self (expected %s)", collection_metatable_key);
      }
      return *self;
   }
   static prop_wrapped_type* unwrap(wrapper& w) {
      auto* prop = wrappers::papyrus_property::unwrap(w);
      if (!dovah::loaded_forms::components::papyrus::property_type_is_array(prop->type()))
         return nullptr;
      return prop;
   }
   static size_t size_of(prop_wrapped_type& prop) {
      size_t size = 0;
      std::visit(
         [&size](auto& v) {
            using value_type = std::decay_t<decltype(v)>;
            if constexpr (cobb::is_std_vector<value_type>) {
               size = v.size();
            }
         },
         prop.value
      );
      return size;
   }

   static void validate_value(prop_wrapped_type& prop, lua_State* L, int pos_value) {
      switch (prop.type()) {
         case property_type::array_of_boolean:
            cobb::lua::argcheck(L, lua_isboolean(L, pos_value), pos_value, "boolean expected");
         case property_type::array_of_float32:
            cobb::lua::argcheck(L, lua_isnumber(L, pos_value), pos_value, "number expected");
         case property_type::array_of_integer:
            cobb::lua::argcheck(L, lua_isinteger(L, pos_value), pos_value, "integer expected");
         case property_type::array_of_string:
            cobb::lua::argcheck(L, lua_isstring(L, pos_value), pos_value, "string expected");
         case property_type::array_of_object:
            if (lua_isnoneornil(L, pos_value))
               break;
            {
               bool array_of_form  = false;
               bool array_of_alias = false;
               {
                  auto& list = std::get<std::vector<property_object_value>>(prop.value);
                  for (auto& item : list) {
                     if (item.alias_id != property_object_value::no_alias) {
                        array_of_alias = true;
                        break;
                     }
                     if (item.form) {
                        array_of_form = true;
                        break;
                     }
                  }
               }
               if (auto* w = wrapper_from_stack<wrappers::quest_alias>(L, pos_value)) {
                  if (!wrappers::quest_alias::unwrap(*w))
                     cobb::lua::argerror(L, pos_value, "provided value is a zombie object");
                  cobb::lua::argcheck(L, !array_of_form, pos_value, "cannot insert a quest alias into an array of forms");
               } else if (auto* w = wrapper_from_stack<wrappers::form>(L, pos_value)) {
                  if (!w->stub)
                     cobb::lua::argerror(L, pos_value, "provided value is a zombie object");
                  cobb::lua::argcheck(L, !array_of_form, pos_value, "cannot insert a form into an array of quest aliases");
               } else {
                  if (array_of_alias)
                     cobb::lua::argerror(L, pos_value, "quest alias expected");
                  if (array_of_form)
                     cobb::lua::argerror(L, pos_value, "form expected");
                  cobb::lua::argerror(L, pos_value, "form or quest alias expected");
               }
            }
            break;
      }
   }
   static void set_value(dovah::loaded_forms::Form& form, prop_wrapped_type& prop, size_t i, lua_State* L, int pos_value) {
      auto& var = prop.value;
      if (auto* casted = std::get_if<std::vector<bool>>(&var)) {
         (*casted)[i] = lua_toboolean(L, pos_value);
      } else if (auto* casted = std::get_if<std::vector<float>>(&var)) {
         (*casted)[i] = lua_tonumber(L, pos_value);
      } else if (auto* casted = std::get_if<std::vector<int32_t>>(&var)) {
         (*casted)[i] = lua_tointeger(L, pos_value);
      } else if (auto* casted = std::get_if<std::vector<std::string>>(&var)) {
         (*casted)[i] = lua_tostring(L, pos_value);
      } else if (auto* casted = std::get_if<std::vector<property_object_value>>(&var)) {
         auto& dst = (*casted)[i];
         if (lua_isnoneornil(L, pos_value)) {
            dst.clear(form);
            return;
         }
         if (auto* w = wrapper_from_stack<wrappers::quest_alias>(L, pos_value)) {
            auto* alias = wrappers::quest_alias::unwrap(*w);
            assert(!!alias);
            dst.form.set(form, &alias->owner.stub);
            dst.alias_id = alias->id;
         } else if (auto* w = wrapper_from_stack<wrappers::form>(L, pos_value)) {
            assert(!!w->stub);
            dst.form.set(form, w->stub);
            dst.alias_id = property_object_value::no_alias;
         }
      }
   }
   
   int get_collection_length(lua_State* L) {
      auto& self = get_collection_wrapper(L);
      auto* prop = unwrap(self);
      if (!prop)
         return 0;
      lua_pushinteger(L, size_of(*prop));
      return 1;
   }
   int lookup_item_by_index(lua_State* L) {
      auto& self = get_collection_wrapper(L);
      auto* prop = unwrap(self);
      if (!prop)
         return 0;

      auto   i    = lua_tointeger(L, 2) - 1;
      size_t size = size_of(*prop);
      if (i < 0 || i >= size)
         return 0;

      if (auto* casted = std::get_if<std::vector<bool>>(&prop->value)) {
         lua_pushboolean(L, (*casted)[i]);
         return 1;
      }
      if (auto* casted = std::get_if<std::vector<float>>(&prop->value)) {
         lua_pushnumber(L, (*casted)[i]);
         return 1;
      }
      if (auto* casted = std::get_if<std::vector<int32_t>>(&prop->value)) {
         lua_pushinteger(L, (*casted)[i]);
         return 1;
      }
      if (auto* casted = std::get_if<std::vector<std::string>>(&prop->value)) {
         lua_pushstring(L, (*casted)[i].c_str());
         return 1;
      }
      if (auto* casted = std::get_if<std::vector<property_object_value>>(&prop->value)) {
         auto& item = (*casted)[i];
         if (!item.form) {
            lua_pushnil(L);
            return 1;
         }
         if (item.alias_id != property_object_value::no_alias) {
            return wrappers::quest_alias::wrap(L, item.form.get_form_stub(), item.alias_id);
         }
         return push_native_object(item.form);
      }
      return 0;
   }
   int member_function_insert(lua_State* L) {
      core::subsystems::permissions::verify_form_write_permissions();
      
      auto& self  = get_collection_wrapper(L);
      auto* prop = unwrap(self);
      if (!prop)
         return 0;

      size_t insert_at = size_of(*prop);
      int    pos_value = 2;
      if (lua_gettop(L) >= 3) {
         pos_value = 3;
         cobb::lua::argcheck(L, lua_isinteger(L, 2), 2, "provided index is not an integer");
         auto i = lua_tointeger(L, 2);
         cobb::lua::argcheck(L, i <= 0, 2, "invalid index");
         insert_at = i - 1;
      }

      validate_value(*prop, L, pos_value);

      self.before_edit();
      auto* form = self.get_loaded_form_data<dovah::loaded_forms::Form>();
      std::visit(
         [L, insert_at](auto& list) {
            using value_type = std::decay_t<decltype(list)>;
            if constexpr (cobb::is_std_vector<value_type>) {
               const auto size = list.size();
               if (insert_at >= size) {
                  if (insert_at > size) {
                     cobb::lua::warning(L, "index %u is out of bounds; nil elements will be created between the end of the list and the new element", (int)insert_at);
                  }
                  list.resize(insert_at + 1);
               } else {
                  list.emplace(list.begin() + insert_at);
               }
            }
         },
         prop->value
      );
      set_value(*form, *prop, insert_at, L, pos_value);
      self.after_edit();

      return 0;
   }
   int member_function_remove(lua_State* L) {
      core::subsystems::permissions::verify_form_write_permissions();

      auto& self = get_collection_wrapper(L);
      auto* prop = unwrap(self);
      if (!prop)
         return 0;

      cobb::lua::argcheck(L, lua_isnumber(L, 2), 2, "integer expected");
      auto i = lua_tointeger(L, 2);
      cobb::lua::argcheck(L, i > 0, 2, "index out of bounds");
      --i;
      cobb::lua::argcheck(L, i < size_of(*prop), 2, "index out of bounds");

      auto* form = self.get_loaded_form_data<dovah::loaded_forms::Form>();
      self.before_edit();
      {
         if (auto* casted = std::get_if<std::vector<property_object_value>>(&prop->value)) {
            (*casted)[i].clear(*form);
         }
         std::visit(
            [i](auto& list) {
               using value_type = std::decay_t<decltype(list)>;
               if constexpr (cobb::is_std_vector<value_type>) {
                  list.erase(list.begin() + i);
               }
            },
            prop->value
         );
      }
      self.after_edit();
      return 0;
   }
   int set_item(lua_State* L) {
      core::subsystems::permissions::verify_form_write_permissions();

      auto& self = get_collection_wrapper(L);
      auto* prop = unwrap(self);
      if (!prop)
         return 0;

      cobb::lua::argcheck(L, lua_isnumber(L, 2), 2, "integer expected");
      auto i = lua_tointeger(L, 2);
      cobb::lua::argcheck(L, i > 0, 2, "index out of bounds");
      --i;
      cobb::lua::argcheck(L, i < size_of(*prop), 2, "index out of bounds");

      validate_value(*prop, L, 3);

      self.before_edit();
      auto* form = self.get_loaded_form_data<dovah::loaded_forms::Form>();
      set_value(*form, *prop, i, L, 3);
      self.after_edit();
      return 0;
   }
}

namespace dovahscript::wrappers::collections {
   extern const collection_definition_params papyrus_array_property_values = {
      .registry_key           = collection_metatable_key,
      .garbage_collection     = &wrapper::__gc,
      //
      .get_collection_length  = &get_collection_length,
      .lookup_item_by_index   = &lookup_item_by_index,
      .member_function_insert = &member_function_insert,
      .member_function_remove = &member_function_remove,
      .set_item               = &set_item,
   };
}