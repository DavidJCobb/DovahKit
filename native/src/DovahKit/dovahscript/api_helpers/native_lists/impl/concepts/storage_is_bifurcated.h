#pragma once
#include <concepts>
#include <utility> // std::pair
namespace dovahscript {
   class wrapper;
}

namespace dovahscript::api_helpers::native_lists::impl::concepts {
   template<typename Spec>
   concept storage_is_bifurcated = requires(wrapper& self) {
      typename Spec::collection_wrapped_type;
      { Spec::unwrap_collection(self) } -> std::same_as<std::pair<typename Spec::collection_wrapped_type*, typename Spec::collection_wrapped_type*>>;
   };
}