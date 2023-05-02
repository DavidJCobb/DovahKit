#include "./options_union.h"
#include <array>

namespace {
   namespace worldedit {
      using namespace ::dovahkit::subsystems::worldedit;
   }

   #pragma region Machinery for: options_union::construct_for_type
   template<typename T> worldedit::tools::options_union default_construct_union() {
      if constexpr (worldedit::tools::tool_with_options_member_type<T>) {
         return worldedit::tools::options_union(typename T::options{});
      } else {
         return worldedit::tools::options_union{};
      }
   }

   template<typename T> struct _types_to_constructors;
   template<typename... Types> struct _types_to_constructors<std::tuple<Types...>> {
      static constexpr const auto value = std::array{ &default_construct_union<Types>... };
   };
   static constexpr const auto union_constructors_by_id = _types_to_constructors<worldedit::tools::all_tools::as_tuple>::value;
   #pragma endregion
}

namespace dovahkit::subsystems::worldedit::tools {
   void options_union::_destroy_data() {
      if (this->tag == id_of_none)
         return;
      (union_data_destructors_by_id[this->tag])(*this);
   }

   /*static*/ options_union options_union::construct_for_type(tool_id id) {
      if (id == id_of_none)
         return {};
      return (union_constructors_by_id[id])();
   }
}