#pragma once
#include <tuple>

namespace cobb::tuples {
   namespace impl {
      template<typename, typename> struct _prepend;
      template<typename Prepend, typename... Prior> struct _prepend<Prepend, std::tuple<Prior...>> {
         using type = std::tuple<Prepend, Prior...>;
      };
   }

   template<typename Prepend, typename Prior> using prepend = impl::_prepend<Prepend, Prior>::type;
}