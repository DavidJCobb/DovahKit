#include "_options.h"

namespace worldinput {
   using namespace ::dovahkit::subsystems::worldinput;
}

namespace {
   #pragma region Machinery for: option_union::construct_for_type
   template<typename T> worldinput::tools::option_union default_construct_union() {
      if constexpr (worldinput::tools::tool_has_options_member_type<T>) {
         return worldinput::tools::option_union(T::options());
      }
      return worldinput::tools::option_union();
   }

   template<typename T> struct _types_to_constructors;
   template<typename... Types> struct _types_to_constructors<std::tuple<Types...>> {
      static constexpr const auto value = std::array{ &default_construct_union<Types>... };
   };
   static constexpr const auto union_constructors_by_id = _types_to_constructors<worldinput::all_tools::as_tuple>::value;
   #pragma endregion
}

namespace dovahkit::subsystems::worldinput::tools {
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