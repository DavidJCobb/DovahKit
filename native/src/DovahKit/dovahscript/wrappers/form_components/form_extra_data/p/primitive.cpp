#include "./primitive.h"
#include <optional>
#include "helpers/lua/error.h"
#include "dovahscript/api_helpers/fail_table_if_expandos.h"
#include "dovahscript/core/subsystems/permissions.h"
#include "dovahscript/core/subsystems/userdata.h"
#include "dovahscript/core/classes.h"
#include "dovahscript/pull_native_object.h"
#include "dovahscript/push_native_object.h"
#include "dovahscript/wrapper.h"

#include "dovah/forms/components/extra_data/types/p/primitive.h"
#include "dovah/forms/components/extra_data.h"
#include "../../form_extra_data.h"
#include "dovahscript/wrappers/form/form.h"

#include "./primitive/primitive_bounds.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::form_extra_data_types::primitive;
   using wrapped_type = cls::wrapped_type;

   namespace extra_data_types {
      using namespace dovah::loaded_forms::components::extra_data_types;
   }
}

wrapped_type* cls::unwrap(wrapper& w) {
   auto* edl = form_extra_data::unwrap(w);
   if (!edl)
      return nullptr;
   return edl->get<cls::wrapped_type>();
}

#define FOR_EACH_PRIMITIVE_SHAPE(X) \
   X(none,       "none") \
   X(box,        "box") \
   X(portal_box, "plane") \
   X(sphere,     "sphere") \
   X(line,       "line")

namespace {
   std::optional<enum wrapped_type::shape> pull_shape(std::string_view view) {
      #define X(_enum, _name, ...) if (view == _name) { return wrapped_type::shape::_enum; }
      FOR_EACH_PRIMITIVE_SHAPE(X);
      #undef X
      return {};
   }
}

void cls::validate_table_for_assign(lua_State* L, int stack_pos) {
   stack_pos = lua_absindex(L, stack_pos);

   if (!lua_istable(L, stack_pos) && !lua_isuserdata(L, stack_pos))
      cobb::lua::argerror(L, stack_pos, "table or userdata expected");
   
   api_helpers::fail_table_if_expandos(L, stack_pos, std::array{
      std::string_view("bounds"),
      std::string_view("shape"),
   });

   {
      lua_getfield(L, stack_pos, "shape");
      if (!lua_isstring(L, -1)) {
         lua_pop(L, 1);
         cobb::lua::argerror(L, stack_pos, "field `shape` must be a string");
      }
      std::string_view v = lua_tostring(L, -1);
      lua_pop(L, 1);
      if (!pull_shape(v).has_value())
         cobb::lua::argerror(L, stack_pos, "field `shape` is not a recognized shape type");
   }
   {
      lua_getfield(L, stack_pos, "bounds");
      if (!lua_istable(L, -1) && !lua_isuserdata(L, -1)) {
         lua_pop(L, 1);
         cobb::lua::argerror(L, stack_pos, "field `bounds` must be a table or userdata");
      }
      for (size_t i = 0; i < 3; ++i) {
         lua_geti(L, -1, i + 1);
         if (lua_isnumber(L, -1)) {
            lua_pop(L, 1);
            continue;
         }
         bool indexed_absent = lua_isnoneornil(L, -1);
         lua_pop(L, 1);
         char name[2] = { 'x' + i, '\0' };
         lua_getfield(L, -1, name);
         if (lua_isnumber(L, -1)) {
            lua_pop(L, 1);
            continue;
         }
         bool named_absent = lua_isnoneornil(L, -1);
         lua_pop(L, 2);
         std::string err;
         if (named_absent) {
            if (indexed_absent) {
               err = std::format("field `bounds` does not have field {} or field '{}'", i + 1, name);
            } else {
               err = std::format("field `bounds` field {} is not a number, and field '{}' is not present", i + 1, name);
            }
         } else {
            if (indexed_absent) {
               err = std::format("field `bounds` does not have field {}, and field '{}' is not a number", i + 1, name);
            } else {
               err = std::format("field `bounds` fields {} and '{}' are not numbers", i + 1, name);
            }
         }
         cobb::lua::argerror(L, stack_pos, err.c_str());
      }
      lua_pop(L, 1);
   }
}
void cls::assign(wrapped_type& dst, lua_State* L, int stack_pos) {
   stack_pos = lua_absindex(L, stack_pos);

   cobb::vector3<float> bounds;
   enum wrapped_type::shape shape = wrapped_type::shape::none;

   {
      lua_getfield(L, stack_pos, "shape");
      auto opt = pull_shape(lua_tostring(L, -1));
      lua_pop(L, 1);
      shape = opt.value();
   }
   {
      lua_getfield(L, stack_pos, "bounds");
      for (size_t i = 0; i < 3; ++i) {
         lua_geti(L, -1, i + 1);
         if (lua_isnumber(L, -1)) {
            bounds[i] = lua_tonumber(L, -1);
            lua_pop(L, 1);
            continue;
         }
         lua_pop(L, 1);
         char name[2] = { 'x' + i, '\0' };
         lua_getfield(L, -1, name);
         assert(lua_isnumber(L, -1) && "this should've been checked earlier, with a call to `validate_table_for_assign`!");
         bounds[i] = lua_tonumber(L, -1);
         lua_pop(L, 1);
      }
      lua_pop(L, 1);
   }

   dst.bounds = bounds;
   dst.shape  = shape;
}

namespace {
   namespace _getters {
      int bounds(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<dovah::loaded_forms::Form>();
         auto* data = cls::unwrap(self);
         if (!data)
            return 0;
         
         wrapper out = self;
         out.append_part(wrapper_part_types::form_extra_data_primitive_bounds);
         return core::subsystems::userdata::get().push(L, out, wrappers::form_extra_data_types::primitive__bounds::metatable_key);
      }
      int shape(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<dovah::loaded_forms::Form>();
         auto* data = cls::unwrap(self);
         if (!data)
            return 0;
         switch (data->shape) {
            #define X(_enum, _name, ...) case wrapped_type::shape::_enum: lua_pushstring(L, _name); return 1;
            FOR_EACH_PRIMITIVE_SHAPE(X);
            #undef X
         }
         return 0;
      }
   }
   namespace _setters {
      int bounds(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<dovah::loaded_forms::Form>();
         auto* data = cls::unwrap(self);
         if (!data)
            return 0;

         // STACK: [self, argument]
         lua_getfield(L, 1, "bounds");
         // STACK: [self, argument, self.bounds]
         lua_getfield(L, -1, "set_xyz");
         // STACK: [self, argument, self.bounds, self.bounds.set_xyz]
         lua_pushvalue(L, -2);
         // STACK: [self, argument, self.bounds, self.bounds.set_xyz, self.bounds]
         lua_pushvalue(L, 2);
         // STACK: [self, argument, self.bounds, self.bounds.set_xyz, self.bounds, argument]
         lua_call(L, 2, 0);
         // STACK: [self, argument, self.bounds]
         return 0;
      }
      int shape(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<dovah::loaded_forms::Form>();
         auto* data = cls::unwrap(self);
         if (!data)
            cobb::lua::error(L, "form-extra-data-primitive wrapper has no underlying object (deleted?)");
         cobb::lua::argcheck(L, lua_isstring(L, 2), 2, "string expected");

         const auto shape_opt = pull_shape(lua_tostring(L, 2));
         if (!shape_opt.has_value())
            cobb::lua::argerror(L, 2, "unrecognized shape name");
         const auto& shape = shape_opt.value();

         self.before_edit();
         data->shape = shape;
         self.after_edit();
         return 0;
      }
   }
}
namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = {
   };
   
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "bounds", &_getters::bounds },
      // TODO: color
      { "shape",  &_getters::shape },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "bounds", &_setters::bounds },
      // TODO: color
      { "shape",  &_setters::shape },
   };
}