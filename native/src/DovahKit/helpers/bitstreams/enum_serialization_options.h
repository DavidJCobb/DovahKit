#pragma once
#include <array>
#include <concepts>
#include <type_traits>

namespace cobb::bitstreams {

   //
   // You can specialize this type in order to define serialization and validation options 
   // for an enum type. Members you can define include:
   // 
   // 
   //    static constexpr const size_t bitcount
   // 
   //       The number of bits that this enum should use when serialized.
   // 
   //
   //    static constexpr const std::array<value_type, ...> valid_values
   // 
   //       A list of all values in this enum that are considered valid for serialization; 
   //       can be used when reading, to throw an exception upon reading a garbage enum.
   // 
   // 
   // Members you must define include:
   // 
   // 
   //    using value_type = ...
   // 
   //       The enum type itself.
   // 
   // 
   // You must specialize this type from outside of any namespace, as MSVC has not fully 
   // implemented P1787, a C++ standard proposal (accepted in November 2020) that allows 
   // you to specialize a template from basically any namespace.
   //
   template<typename Enum> requires std::is_enum_v<Enum>
   struct enum_serialization_options {
      enum_serialization_options() = delete;

      using value_type = Enum;
   };

}