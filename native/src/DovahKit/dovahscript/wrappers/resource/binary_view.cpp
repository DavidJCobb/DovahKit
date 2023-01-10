#include "binary_view.h"
#include <bit>
#include "../../../helpers/lua/error.h"
#include "../../../helpers/lua/set_top_on_exit.h"
#include "../../../helpers/lua/tostringex.h"
#include "../../../helpers/lua/warning.h"
#include "../../../helpers/endian.h"
#include "../../core/subsystems/permissions.h"
#include "../../core/subsystems/resources.h"
#include "../../push_native_object.h"
#include "../../send_script_task.h"
#include "../../task_reference.h"

#include "../../tasks/s2m/lambda.h"
#include "../../tasks/s2m/ui_read_lambda.h"
#include "../../tasks/s2m/ui_write_lambda.h"

namespace {
   constexpr size_t max_synthetic_size = 1024 * 1024 * 1024; // 1GB
}

namespace {
   using namespace dovahscript;
   using cls = wrappers::resource::binary_view;

   namespace _helpers {
      template<typename T> std::endian get_endianness(lua_State* L, int pos, int arg) {
         if (sizeof(T) == 1)
            return std::endian::little;
         if (!lua_isnoneornil(L, pos)) {
            if (lua_isstring(L, pos)) {
               auto* s = lua_tostring(L, pos);
               if (_stricmp(s, "big") == 0)
                  return std::endian::big;
               if (_stricmp(s, "little") == 0)
                  return std::endian::little;
               cobb::lua::argerror(L, arg, "unrecognized endianness value");
            }
            cobb::lua::argerror(L, arg, "endianness (string) or nil expected");
         }
         return std::endian::little;
      }

      template<typename T> size_t get_offset(lua_State* L, int pos, int arg, size_t buffer_size) {
         int    isnum;
         size_t offset = lua_tointegerx(L, pos, &isnum);
         cobb::lua::argcheck(L, isnum,       arg, "offset (integer) expected");
         cobb::lua::argcheck(L, offset >= 0, arg, "offset cannot be negative");
         cobb::lua::argcheck(L, offset < buffer_size,              arg, "offset exceeds the bounds of the data view");
         cobb::lua::argcheck(L, offset + sizeof(T) <= buffer_size, arg, "the desired value would extend past the bounds of the data view");
         return offset;
      }
   }

   namespace _methods {
      template<typename T> int _read(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.managed_resource)
            return 0;
         int arg_offset = 2;
         int arg_endian = 3;
         if (lua_istable(L, 2) || lua_isuserdata(L, 2)) {
            lua_settop(L, 2);
            lua_getfield(L, 2, "offset");
            lua_getfield(L, 2, "endian");
            lua_remove(L, 2);
            arg_offset = 2;
            arg_endian = 2;
         }
         const auto buffer = self.managed_resource->get_binary_script_side(); // implicitly shared, so no worry about copying
         auto       offset = _helpers::get_offset<T>(L, 2, arg_offset, buffer.size());
         auto       endian = _helpers::get_endianness<T>(L, 3, arg_endian);
         //
         // Read:
         //
         auto* raw   = (const uint8_t*)buffer.constData();
         auto  value = *(const T*)(raw + offset);
         if constexpr (sizeof(T) > 1) {
            value = cobb::endian_cast(endian, value);
         }
         if constexpr (std::is_same_v<T, bool>) {
            lua_pushboolean(L, value);
         } else if constexpr (std::is_integral_v<T>) {
            lua_pushinteger(L, value);
         } else if constexpr (std::is_floating_point_v<T>) {
            lua_pushnumber(L, value);
         }
         return 1;
      }
      template<typename T> int _write(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.managed_resource)
            return 0;
         int arg_value  = 2;
         int arg_offset = 3;
         int arg_endian = 4;
         if (lua_istable(L, 2) || lua_isuserdata(L, 2)) {
            lua_settop(L, 2);
            lua_getfield(L, 2, "value");
            lua_getfield(L, 2, "offset");
            lua_getfield(L, 2, "endian");
            lua_remove(L, 2);
            arg_value  = 2;
            arg_offset = 2;
            arg_endian = 2;
         }
         //
         T value;
         if constexpr (std::is_same_v<T, bool>) {
            cobb::lua::argcheck(L, lua_isboolean(L, 2), arg_value, "value (boolean) expected");
            value = lua_toboolean(L, 2);
         } else if constexpr (std::is_integral_v<T>) {
            int isnum;
            lua_Integer v = lua_tointegerx(L, 2, &isnum);
            cobb::lua::argcheck(L, isnum, arg_value, "value (integer) expected");
            cobb::lua::argcheck(L, v >= std::numeric_limits<T>::min(), arg_value, "the specified value is below the minimum and would overflow");
            cobb::lua::argcheck(L, v <= std::numeric_limits<T>::max(), arg_value, "the specified value is below the maximum and would overflow");
            value = v;
         } else if constexpr (std::is_floating_point_v<T>) {
            cobb::lua::argcheck(L, lua_isnumber(L, 2), arg_value, "value (number) expected");
            value = lua_tonumber(L, 2);
         }
         //
         const auto buffer = self.managed_resource->get_binary_script_side();
         auto offset = _helpers::get_offset<T>(L, 3, arg_offset, buffer.size());
         auto endian = _helpers::get_endianness<T>(L, 4, arg_endian);
         //
         // Write:
         //
         if constexpr (sizeof(T) > 1) {
            value = cobb::endian_cast(endian, value);
         }
         self.managed_resource->modify_binary_script_side([value, offset](QByteArray& buffer) {
            auto* raw = (uint8_t*)buffer.data();
            *(T*)(raw + offset) = value;
         });
         return 0;
      }
      template<typename T> int _append(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.managed_resource)
            return 0;
         int arg_value  = 2;
         int arg_endian = 3;
         if (lua_istable(L, 2) || lua_isuserdata(L, 2)) {
            lua_settop(L, 2);
            lua_getfield(L, 2, "value");
            lua_getfield(L, 2, "endian");
            lua_remove(L, 2);
            arg_value  = 2;
            arg_endian = 2;
         }
         //
         T value;
         if constexpr (std::is_same_v<T, bool>) {
            cobb::lua::argcheck(L, lua_isboolean(L, 2), arg_value, "value (boolean) expected");
            value = lua_toboolean(L, 2);
         } else if constexpr (std::is_integral_v<T>) {
            int isnum;
            lua_Integer v = lua_tointegerx(L, 2, &isnum);
            cobb::lua::argcheck(L, isnum, arg_value, "value (integer) expected");
            cobb::lua::argcheck(L, v >= std::numeric_limits<T>::min(), arg_value, "the specified value is below the minimum and would overflow");
            cobb::lua::argcheck(L, v <= std::numeric_limits<T>::max(), arg_value, "the specified value is below the maximum and would overflow");
            value = v;
         } else if constexpr (std::is_floating_point_v<T>) {
            cobb::lua::argcheck(L, lua_isnumber(L, 2), arg_value, "value (number) expected");
            value = lua_tonumber(L, 2);
         }
         //
         auto endian = _helpers::get_endianness<T>(L, 3, arg_endian);
         //
         // Write:
         //
         if constexpr (sizeof(T) > 1) {
            value = cobb::endian_cast(endian, value);
         }
         self.managed_resource->modify_binary_script_side([value](QByteArray& buffer) {
            buffer.resize(buffer.size() + sizeof(T));
            auto* raw = (uint8_t*)buffer.data();
            *(T*)(raw + buffer.size() - sizeof(T)) = value;
         });
         return 0;
      }
      //
      int reserve(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.managed_resource)
            return 0;
         int    isnum;
         size_t size = lua_tointegerx(L, 2, &isnum);
         cobb::lua::argcheck(L, isnum,     2, "size (integer) expected");
         cobb::lua::argcheck(L, size >= 0, 2, "the size cannot be negative");
         auto buffer = self.managed_resource->get_binary_script_side();
         if (size <= buffer.size())
            return 0;
         if (size > max_synthetic_size)
            size = max_synthetic_size;
         self.managed_resource->reserve_binary_script_side(size);
         return 0;
      }
      int resize(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.managed_resource)
            return 0;
         int    isnum;
         size_t size = lua_tointegerx(L, 2, &isnum);
         cobb::lua::argcheck(L, isnum,     2, "size (integer) expected");
         cobb::lua::argcheck(L, size >= 0, 2, "the size cannot be negative");
         auto buffer = self.managed_resource->get_binary_script_side();
         if (size > buffer.size()) {
            cobb::lua::argcheck(L, size <= max_synthetic_size, 2, "max size limit: you cannot resize a buffer to above 1GB unless it was already huge (e.g. loaded from a file) and you are making it smaller");
         }
         if (size == buffer.size())
            return 0;
         self.managed_resource->modify_binary_script_side([size](QByteArray& buffer) {
            auto prior = buffer.size();
            buffer.resize(size);
            buffer.squeeze();
            //
            if (prior < size) {
               auto diff = size - prior;
               memset(buffer.data() + prior, 0, diff);
            }
         });
         return 0;
      }
   }
   namespace _getters {
      int size(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.managed_resource)
            return 0;
         const auto buffer = self.managed_resource->get_binary_script_side(); // implicitly shared, so no worry about copying
         lua_pushinteger(L, buffer.size());
         return 1;
      }
   }
   namespace _setters {
      int size(lua_State* L) {
         return _methods::resize(L);
      }
   }

   namespace _singleton_functions {
      int new_(lua_State* L) {
         size_t initial = 0;
         {
            int isnum;
            initial = lua_tointegerx(L, 1, &isnum);
            if (!isnum)
               initial = 0;
            cobb::lua::argcheck(L, initial <= max_synthetic_size, 2, "max size limit: you cannot create a buffer larger than 1GB");
         }
         task_reference<DovahscriptResource> resource;
         {
            auto* task    = new tasks::s2m::lambda(true);
            task->handler = [&resource, initial]() {
               resource = core::subsystems::resources::get().create_resource(QByteArray(initial, 0), resource_type::binary);
            };
            send_script_task(*task);
            delete task;
            //
            if (!resource)
               cobb::lua::error(L, "unable to create a binary_view for %u bytes", initial);
         }
         return push_native_object(resource);
      }
      int is(lua_State* L) {
         auto* wrapper = wrapper_from_stack<cls>(L, 1);
         lua_pushboolean(L, wrapper != nullptr);
         return 1;
      }
   }
}

namespace dovahscript::wrappers::resource {
   /*static*/ cls::method_list_t cls::metatable_methods = {
      { "get_bool",    &_methods::_read<bool> },
      { "get_double",  &_methods::_read<double> },
      { "get_float",   &_methods::_read<float> },
      { "get_int8",    &_methods::_read<int8_t> },
      { "get_int16",   &_methods::_read<int16_t> },
      { "get_int32",   &_methods::_read<int32_t> },
      { "get_uint8",   &_methods::_read<uint8_t> },
      { "get_uint16",  &_methods::_read<uint16_t> },
      { "get_uint32",  &_methods::_read<uint32_t> },
      //
      { "set_bool",    &_methods::_write<bool> },
      { "set_double",  &_methods::_write<double> },
      { "set_float",   &_methods::_write<float> },
      { "set_int8",    &_methods::_write<int8_t> },
      { "set_int16",   &_methods::_write<int16_t> },
      { "set_int32",   &_methods::_write<int32_t> },
      { "set_uint8",   &_methods::_write<uint8_t> },
      { "set_uint16",  &_methods::_write<uint16_t> },
      { "set_uint32",  &_methods::_write<uint32_t> },
      //
      { "append_bool",    &_methods::_append<bool> },
      { "append_double",  &_methods::_append<double> },
      { "append_float",   &_methods::_append<float> },
      { "append_int8",    &_methods::_append<int8_t> },
      { "append_int16",   &_methods::_append<int16_t> },
      { "append_int32",   &_methods::_append<int32_t> },
      { "append_uint8",   &_methods::_append<uint8_t> },
      { "append_uint16",  &_methods::_append<uint16_t> },
      { "append_uint32",  &_methods::_append<uint32_t> },
      //
      { "reserve", &_methods::reserve },
      { "resize",  &_methods::resize },
   };
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "size", &_getters::size },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "size", &_setters::size },
   };

   /*static*/ void cls::import_singleton(lua_State* L) {
      lua_createtable(L, 0, 2);
      lua_pushcfunction(L, &_singleton_functions::new_);
      lua_setfield     (L, -2, "new");
      lua_pushcfunction(L, &_singleton_functions::is);
      lua_setfield     (L, -2, "is");
   }
}