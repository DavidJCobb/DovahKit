#pragma once
#include <concepts>
#include <type_traits>

namespace cobb::bitstreams {
   template<size_t Bitcount, typename T>
   struct stream_with_bitcount {
      static constexpr const size_t bitcount = Bitcount;
      using value_type = T;

      T& target;
   };

   namespace impl::_stream_with_bitcount {
      template<typename T> concept writable_specialization = requires {
         typename T::value_type;
         requires std::is_const_v<typename T::value_type>;
         { T::bitcount } -> std::same_as<const size_t&>;
         requires std::is_same_v<T, stream_with_bitcount<T::bitcount, typename T::value_type>>;
      };
      
      template<typename T> concept readable_specialization = requires {
         typename T::value_type;
         requires !std::is_const_v<typename T::value_type>;
         { T::bitcount } -> std::same_as<const size_t&>;
         requires std::is_same_v<T, stream_with_bitcount<T::bitcount, typename T::value_type>>;
      };
   }
}