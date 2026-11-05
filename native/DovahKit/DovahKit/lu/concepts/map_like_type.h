#pragma once
#include <concepts>
#include <type_traits>
#include <utility> // std::pair
#include "./supports_range_based_for_loops.h"

namespace lu::concepts {
   template<typename T>
   concept map_like_type = requires {
      requires supports_range_based_for_loops<T>;
      typename T::iterator;
      typename T::key_type;
      typename T::mapped_type;
      typename T::value_type;
      requires std::is_same_v<typename T::value_type, std::pair<typename T::key_type, typename T::mapped_type>>;
      requires requires (
         T& map,
         const T& cmap,
         typename T::iterator it,
         typename T::const_iterator cit,
         typename T::key_type k
      ) {
         { *it  } -> std::same_as<typename T::value_type&>;
         { *cit } -> std::same_as<const typename T::value_type&>;
         { map[k]  } -> std::same_as<typename T::mapped_type&>;
         { cmap[k] } -> std::same_as<const typename T::mapped_type&>;
      };
   };
}