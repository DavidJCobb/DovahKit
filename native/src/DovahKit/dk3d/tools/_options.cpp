#include "_options.h"

namespace {
   #pragma region Machinery for: option_union::construct_for_type
   template<typename T> DK3D::tools::option_union default_construct_union() {
      if constexpr (tool_has_options_member_type<T>) {
         return DK3D::tools::option_union(T::options());
      }
      return DK3D::tools::option_union();
   }
   using option_union_constructor = DK3D::tools::option_union(*)();

   template<typename T> struct _types_to_constructors;
   template<typename... Types> struct _types_to_constructors<std::tuple<Types...>> {
      static constexpr size_t size = sizeof...(Types);

      static std::array<option_union_constructor, size> value = { &default_construct_union<Types>, ... };
   };
   static const auto union_constructors_by_id = _types_to_constructors<DK3D::all_tools::as_tuple>::value;
   #pragma endregion

   #pragma region Machinery for: option_union::_destroy_data
   template<typename T> void destroy_union_data(DK3D::tools::option_union& ou) {
      if constexpr (tool_has_options_member_type<T>) {
         auto* p = (T*)ou.data.data();
         p->~T();
      }
   }
   template<typename T> struct _types_to_destructors;
   template<typename... Types> struct _types_to_destructors<std::tuple<Types...>> {
      static constexpr size_t size = sizeof...(Types);

      static std::array<decltype(destroy_union_data<void>), size> value = { &destroy_union_data<Types>, ... };
   };
   static const auto union_data_destructors_by_id = _types_to_destructors<DK3D::all_tools::as_tuple>::value;
   #pragma endregion
}

namespace DK3D::tools {
   void option_union::_destroy_data() {
      if (this->tag == id_of_none)
         return;
      (union_data_destructors_by_id[this->tag])(*this);
   }

   /*static*/ option_union option_union::construct_for_type(tool_id id) {
      if (id == id_of_none)
         return option_union();
      return (union_constructors_by_id[id])();
   }
}