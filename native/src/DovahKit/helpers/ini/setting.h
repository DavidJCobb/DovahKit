#pragma once
#include <limits>
#include <variant>
#include "./config.h"
#include "./types.h"
#include "./value_constraint_info.h"
#include "./setting_definition.h"

namespace cobb::ini {
   class category;
}

namespace cobb::ini {
   // Instances should use the `constinit` specifier.
   class setting {
      public:
         template<typename T>
         struct value_info {
            T current;
            T initial;
         };

         template<typename T>
         struct info_type {
            value_info<T> values;
            [[no_unique_address]] value_constraint_info<T> constraints;
         };

      public:
         category& owner;
         const char* const name;
         
      protected:
         value_types::unpack_wrapped_types_into<std::variant, info_type> info;

         // A typical constructor; it performs no compile-time validation of data. 
         // Use the static `setting::define` function, templated on an NTTP setting 
         // definition, to instantiate settings with compile-time validation of the 
         // definition you provide.
         template<typename T>
         constexpr setting(category&, const setting_definition<T>&); // intentionally not public

      public:
         // A static member function which takes a setting definition as a non-type 
         // template parameter and returns a newly-created setting. Said definition 
         // is checked for validity and consistency at compile-time.
         template<auto Definition> requires (setting_definition_type<std::decay_t<decltype(Definition)>> && (prefer_static_assertions || Definition.is_valid()))
         static constexpr setting define(category&);

         template<typename T>
         constexpr bool is_of_type() const noexcept;

         // Throws if the setting is not of the requested type.
         template<typename T> requires value_types::has_key<T>
         constexpr const value_constraint_info<T>& get_constraints() const;

         // Throws if the setting is not of the requested type.
         template<typename T> requires value_types::has_key<T>
         constexpr T get_initial_value() const;

         // Throws if the setting is not of the requested type.
         template<typename T> requires value_types::has_key<T>
         constexpr T get_current_value() const;

         constexpr value_variant get_current_value_variant() const;

         // Throws if the setting is not of the requested type.
         // Throws if the value supplied does not satisfy the constraints.
         template<typename T> requires value_types::has_key<T>
         constexpr void set_current_value(const T& v) const;

         // This is provided in order to implement serialization and parsing of INI 
         // files. Its use outside of that task is discouraged.
         template<bool Dummy = true>
         constexpr void set_current_value_variant(const value_variant& v);
   };
}

#include "./setting.inl"