#pragma once
#include <type_traits>

namespace dovah::loaded_forms {
   namespace components {
      namespace papyrus {
         class script_data;
      }
   }

   namespace impl {
      /*// How to test for the existence of a member named "foo" on type T:
      template<typename, class = void> struct has_foo : std::false_type {};
      template<typename T> struct has_foo<T, std::void_t<decltype(T::foo)>> {};
      //
      if constexpr (has_foo<x>::value) {}
      */
      
      #pragma region get_papyrus_data(Form&)
      template<typename T, typename = void> struct _get_papyrus_data_impl : std::false_type {
         inline static components::papyrus::script_data* func(T&) { return nullptr; }
      };
      template<typename T> struct _get_papyrus_data_impl<T, std::void_t<decltype(T::script_data)>> {
         inline static components::papyrus::script_data* func(T& v) { return &v.script_data; }
      };
      //
      template<typename T> inline components::papyrus::script_data* get_papyrus_data(T& v) {
         return _get_papyrus_data_impl<T>::func(v);
      }
      #pragma endregion
   }
}
