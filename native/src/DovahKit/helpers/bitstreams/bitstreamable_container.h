#pragma once
#include <type_traits>
#include <utility> // std::pair
#include "./bitstreamable_struct.h"
#include "./bitstreamable_primitive.h"

namespace cobb {
   namespace impl::_bitstreamable_container {
      template<typename T> concept is_map = requires {
         typename T::key_type;
         typename T::mapped_type;
         typename T::value_type;
         requires std::is_same_v<typename T::value_type, std::pair<const typename T::key_type, typename T::mapped_type>>;
      };
   }

   template<typename T> concept bitstreamable_container = requires {
      requires !impl::_bitstreamable_container::is_map<T>;

      typename T::value_type;
      requires bitstreamable_struct<typename T::value_type> || bitstreamable_primitive<typename T::value_type>;
      requires requires(T& x) {
         { x.begin() };
         { x.end() };
         { x.begin() == x.end() };
         { (x.begin())++ };
         { *(x.begin()) };
      };
   };
}
