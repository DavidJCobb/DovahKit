#include "cubemap_face_list.h"
#include "image.h"
#include "../dds.h"
#include "../raster.h"

#include "../../../systems/lua_managed_resources.h"
#include "../../../systems/userdata.h"

#include "../../../editor_script_core.h"
#include "../../../wrapper_util.h"

#include "../../../cross_thread_tasks/s2m/lambda.h"

namespace {
   static constexpr std::array<cobb::eight_cc, 6> _cubemap_face_parts = {{
      editor_script::wrapper_part_types::resource_dds_cubemap_face_x_pos,
      editor_script::wrapper_part_types::resource_dds_cubemap_face_x_neg,
      editor_script::wrapper_part_types::resource_dds_cubemap_face_y_pos,
      editor_script::wrapper_part_types::resource_dds_cubemap_face_y_neg,
      editor_script::wrapper_part_types::resource_dds_cubemap_face_z_pos,
      editor_script::wrapper_part_types::resource_dds_cubemap_face_z_neg,
   }};
}

namespace {
   using namespace editor_script;
   using cls = wrappers::resource::dds_cubemap_face_list;

   namespace _methods {
   }
   namespace _getters {
      luastackchange_t x_pos(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.managed_resource)
            return 0;
         wrapper out = self;
         out.last_part().signature = wrapper_part_types::resource_dds_cubemap_face_x_pos;
         out.is_collection = false;
         return DovahKitScriptVMUserdataInterface::get().push(L, out, wrappers::resource::dds_image_subresource::metatable_key);
      }
      luastackchange_t x_neg(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.managed_resource)
            return 0;
         wrapper out = self;
         out.last_part().signature = wrapper_part_types::resource_dds_cubemap_face_x_neg;
         out.is_collection = false;
         return DovahKitScriptVMUserdataInterface::get().push(L, out, wrappers::resource::dds_image_subresource::metatable_key);
      }
      luastackchange_t y_pos(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.managed_resource)
            return 0;
         wrapper out = self;
         out.last_part().signature = wrapper_part_types::resource_dds_cubemap_face_y_pos;
         out.is_collection = false;
         return DovahKitScriptVMUserdataInterface::get().push(L, out, wrappers::resource::dds_image_subresource::metatable_key);
      }
      luastackchange_t y_neg(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.managed_resource)
            return 0;
         wrapper out = self;
         out.last_part().signature = wrapper_part_types::resource_dds_cubemap_face_y_neg;
         out.is_collection = false;
         return DovahKitScriptVMUserdataInterface::get().push(L, out, wrappers::resource::dds_image_subresource::metatable_key);
      }
      luastackchange_t z_pos(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.managed_resource)
            return 0;
         wrapper out = self;
         out.last_part().signature = wrapper_part_types::resource_dds_cubemap_face_z_pos;
         out.is_collection = false;
         return DovahKitScriptVMUserdataInterface::get().push(L, out, wrappers::resource::dds_image_subresource::metatable_key);
      }
      luastackchange_t z_neg(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.managed_resource)
            return 0;
         wrapper out = self;
         out.last_part().signature = wrapper_part_types::resource_dds_cubemap_face_z_neg;
         out.is_collection = false;
         return DovahKitScriptVMUserdataInterface::get().push(L, out, wrappers::resource::dds_image_subresource::metatable_key);
      }
   }
   namespace _setters {
   }
}

namespace editor_script::wrappers::resource {
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_methods = {
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_getters = {
      { "x_pos", &_getters::x_pos },
      { "x_neg", &_getters::x_neg },
      { "y_pos", &_getters::y_pos },
      { "y_neg", &_getters::y_neg },
      { "z_pos", &_getters::z_pos },
      { "z_neg", &_getters::z_neg },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = {
   };
}