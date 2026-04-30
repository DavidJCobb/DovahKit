#pragma once
#include <type_traits>
namespace dovah::loaded_forms {
   class Form;
}

namespace dovahscript::api_helpers::native_lists::impl::fields::store_value {
   template<typename Spec>
   concept present = requires {
      { Spec::store_value };
   };
   template<typename Spec>
   concept valid = requires {
      requires present<Spec>;
      typename Spec::value_stored_type;
      typename Spec::value_working_type;
      requires (
         std::is_invocable_v<decltype(Spec::store_value), const typename Spec::value_working_type&, typename Spec::value_stored_type&>
      || std::is_invocable_v<decltype(Spec::store_value), const typename Spec::value_working_type&, typename Spec::value_stored_type&, dovah::loaded_forms::Form&>
      );
   };
}