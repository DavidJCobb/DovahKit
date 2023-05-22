#pragma once
#include <concepts>
#include <type_traits>
#include "./bitstreamable_primitive.h"

namespace cobb::bitstreams {
   template<size_t Bitcount>
   struct override_bitcount {
      override_bitcount() = delete;
      ~override_bitcount() = delete;

      template<bitstreamable_primitive T> requires (!std::is_floating_point_v<T>)
      struct of {
         static constexpr const size_t bitcount = Bitcount;
         using value_type = T;

         value_type& target;
      };

      // deduction guide
      template <typename T> of(T&) -> of<T>;
   };

   //
   // Usage:
   // 
   //    my_reader.stream( override_bitcount<4>::of(variable) );
   //    my_writer.stream( override_bitcount<4>::of(variable) );
   //

   namespace impl::_override_bitcount {
      template<typename T> concept writable_specialization = requires {
         typename T::value_type;
         { T::bitcount } -> std::same_as<const size_t&>;
         requires std::is_same_v<T, typename override_bitcount<T::bitcount>::template of<typename T::value_type>>;
      };
      
      template<typename T> concept readable_specialization = requires {
         typename T::value_type;
         requires !std::is_const_v<typename T::value_type>;
         { T::bitcount } -> std::same_as<const size_t&>;
         requires std::is_same_v<T, typename override_bitcount<T::bitcount>::template of<typename T::value_type>>;
      };
   }
}