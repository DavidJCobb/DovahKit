#include "cubemap_face_list.h"
#include "../../../core/subsystems/userdata.h"

#include "image.h"
#include "../dds.h"
#include "../raster.h"

namespace {
   static constexpr std::array<cobb::eight_cc, 6> _cubemap_face_parts = {{
      dovahscript::wrapper_part_types::resource_dds_cubemap_face_x_pos,
      dovahscript::wrapper_part_types::resource_dds_cubemap_face_x_neg,
      dovahscript::wrapper_part_types::resource_dds_cubemap_face_y_pos,
      dovahscript::wrapper_part_types::resource_dds_cubemap_face_y_neg,
      dovahscript::wrapper_part_types::resource_dds_cubemap_face_z_pos,
      dovahscript::wrapper_part_types::resource_dds_cubemap_face_z_neg,
   }};
}

namespace {
   using namespace dovahscript;
   using cls = wrappers::resource::dds_cubemap_face_list;

   namespace _methods {
   }
   namespace _getters {
      int x_pos(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.managed_resource)
            return 0;
         wrapper out = self;
         out.last_part().signature = wrapper_part_types::resource_dds_cubemap_face_x_pos;
         out.is_collection = false;
         return core::subsystems::userdata::get().push(L, out, wrappers::resource::dds_image_subresource::metatable_key);
      }
      int x_neg(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.managed_resource)
            return 0;
         wrapper out = self;
         out.last_part().signature = wrapper_part_types::resource_dds_cubemap_face_x_neg;
         out.is_collection = false;
         return core::subsystems::userdata::get().push(L, out, wrappers::resource::dds_image_subresource::metatable_key);
      }
      int y_pos(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.managed_resource)
            return 0;
         wrapper out = self;
         out.last_part().signature = wrapper_part_types::resource_dds_cubemap_face_y_pos;
         out.is_collection = false;
         return core::subsystems::userdata::get().push(L, out, wrappers::resource::dds_image_subresource::metatable_key);
      }
      int y_neg(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.managed_resource)
            return 0;
         wrapper out = self;
         out.last_part().signature = wrapper_part_types::resource_dds_cubemap_face_y_neg;
         out.is_collection = false;
         return core::subsystems::userdata::get().push(L, out, wrappers::resource::dds_image_subresource::metatable_key);
      }
      int z_pos(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.managed_resource)
            return 0;
         wrapper out = self;
         out.last_part().signature = wrapper_part_types::resource_dds_cubemap_face_z_pos;
         out.is_collection = false;
         return core::subsystems::userdata::get().push(L, out, wrappers::resource::dds_image_subresource::metatable_key);
      }
      int z_neg(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.managed_resource)
            return 0;
         wrapper out = self;
         out.last_part().signature = wrapper_part_types::resource_dds_cubemap_face_z_neg;
         out.is_collection = false;
         return core::subsystems::userdata::get().push(L, out, wrappers::resource::dds_image_subresource::metatable_key);
      }
   }
   namespace _setters {
   }
}

namespace dovahscript::wrappers::resource {
   /*static*/ cls::method_list_t cls::metatable_methods = {
   };
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "x_pos", &_getters::x_pos },
      { "x_neg", &_getters::x_neg },
      { "y_pos", &_getters::y_pos },
      { "y_neg", &_getters::y_neg },
      { "z_pos", &_getters::z_pos },
      { "z_neg", &_getters::z_neg },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
   };
}