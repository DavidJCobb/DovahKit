#pragma once
#include <cstdint>
#include <optional>
#include <string_view>
#include <variant>
#include "../../form_types.h"
#include "./parameter_underlying_type.h"

namespace dovah::conditions {
   // Helpers for defining the enumerations.
   namespace parameter_types::_enums {
      struct enum_member {
         constexpr enum_member() {}
         constexpr enum_member(const char* n) : name(n) {}
         constexpr enum_member(std::string_view n) : name(n) {}
         constexpr enum_member(int32_t v, std::string_view n) : value(v), name(n) {}
         constexpr enum_member(uint32_t v, std::string_view n) : value(v), name(n) {}

         std::string_view name;
         int32_t          value = 0;
      };

      template<size_t Size>
      class members : public std::array<enum_member, Size> {
         public:
            template<typename... Types>
            constexpr members(Types... names) {
               size_t i = 0;
               (
                  (
                     (*this)[i] = enum_member((int32_t)i, names),
                     ++i
                  )
                  ,
                  ...
               );
            };
      };
      //
      template<typename... Types>
      members(Types...) -> members<sizeof...(Types)>;
   }

   struct parameter_typeinfo {
      public:
         using enum_member = parameter_types::_enums::enum_member;
         using union_decider_function = const parameter_typeinfo* (*)(enumeration_parameter_value decider_dword_value);

      public:
         struct enumeration_data {
            const  enum_member* members = nullptr;
            size_t size = 0;
         };
         struct form_type_list_info {
            const form_type* types = nullptr;
            size_t           size  = 0;
         };
         struct union_info {
            const parameter_typeinfo* decide_by = nullptr;
            union_decider_function    decider   = nullptr;
         };

      public:
         #pragma region Constructors
         constexpr parameter_typeinfo(const char* name, parameter_underlying_type underlying_type) : name(name), underlying_type(underlying_type) {}

         template<size_t Size>
         constexpr parameter_typeinfo(const char* name, const std::array<enum_member, Size>& list)
            :
            name(name),
            underlying_type(parameter_underlying_type::enumeration),
            enumeration_info(enumeration_data{
               .members = list.data(),
               .size    = list.size(),
            })
         {}
         
         template<size_t Size>
         constexpr parameter_typeinfo(const char* name, const std::array<form_type, Size>& list, bool allow_type_overrides = false)
            :
            name(name),
            underlying_type(parameter_underlying_type::form),
            allow_type_overrides(allow_type_overrides),
            allowed_form_types(form_type_list_info{
               .types = list.data(),
               .size  = list.size(),
            })
         {}

         constexpr parameter_typeinfo(const char* name, form_type type, bool allow_type_overrides = false)
            :
            name(name),
            underlying_type(parameter_underlying_type::form),
            allow_type_overrides(allow_type_overrides),
            allowed_form_types(type)
         {}

         static constexpr parameter_typeinfo make_character_enum(const char* name, const char* charset) {
            parameter_typeinfo out(name, parameter_underlying_type::character);
            out.character_values = charset;
            return out;
         }

         static constexpr parameter_typeinfo make_union(const char* name, const parameter_typeinfo& decided_by, union_decider_function decider) {
            parameter_typeinfo out(name, parameter_underlying_type::character);
            out.union_decider = union_info{
               .decide_by = &decided_by,
               .decider   = decider,
            };
            return out;
         }
         #pragma endregion

      public:
         const char*               name;
         parameter_underlying_type underlying_type;

         bool allow_type_overrides = false; // is this type subject to the "Use Aliases" and "Use Package Data" flags on a condition?
         std::variant<
            std::monostate,     // typeinfo is not for forms OR it's for forms but places no constraints on the form type
            form_type,          // single
            form_type_list_info // multiple
         > allowed_form_types;
         std::string_view                character_values; // for parameter_underlying_type::character
         std::optional<enumeration_data> enumeration_info;
         std::optional<union_info>       union_decider;

         constexpr const bool allows_char(char v) const {
            if (this->underlying_type != parameter_underlying_type::character)
               return false;
            return this->character_values.contains(v);
         }
         constexpr const bool allows_form_type(form_type ft) const {
            if (this->underlying_type != parameter_underlying_type::form)
               return false;
            const auto& aft = this->allowed_form_types;
            if (std::holds_alternative<std::monostate>(aft))
               return true;
            if (std::holds_alternative<dovah::form_type>(aft))
               return std::get<form_type>(aft) == ft;
            if (std::holds_alternative<form_type_list_info>(aft)) {
               const auto& list = std::get< form_type_list_info>(aft);
               for (size_t i = 0; i < list.size; ++i)
                  if (list.types[i] == ft)
                     return true;
            }
            return false;
         }

         constexpr const bool is_union() const noexcept {
            return this->union_decider.has_value();
         }
         constexpr const parameter_typeinfo* resolve_union_type(const parameter_typeinfo& previous_argument_type, enumeration_parameter_value previous_argument_value) const {
            auto& decider = this->union_decider.value();
            if (&previous_argument_type != decider.decide_by)
               return nullptr;
            return (decider.decider)(previous_argument_value);
         }

         template<typename Functor>
         void for_each_allowed_form_type(Functor&& functor) const {
            const auto& aft = this->allowed_form_types;
            if (std::holds_alternative<form_type>(aft)) {
               functor(std::get<form_type>(aft));
            } else if (std::holds_alternative<form_type_list_info>(aft)) {
               auto& list = std::get<form_type_list_info>(aft);
               for (size_t i = 0; i < list.size; ++i)
                  functor(list.types[i]);
            }
         }
   };

   namespace parameter_types {
      // Inline to ensure that all references to this use the same address, rather than the compiler 
      // creating multiple copies of it when it's referenced from multiple headers/elsewhere.
      inline constexpr const auto None = parameter_typeinfo("None", parameter_underlying_type::none);
   }
}
