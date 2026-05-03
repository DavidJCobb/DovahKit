
namespace dovah {
   enum class form_data_management_mode {
      // `form_use` is a bare `dovah::form_stub*`
      // `localized_string_use` is a variant of:
      //   - `std::pair<std::string, encoding>`
      //   - l-string file pointer/handle and l-string index
      //   - l-string data (i.e. string+encoding per language)
      unmanaged,
      
      // `form_use` is a `managed_form_use`
      // `localized_string_use` is a `managed_localized_string_use`
      managed,
   };
   
   // Struct in case we need to extend it later
   struct default_form_data_params {
      public:
         form_data_management_mode management_mode = form_data_management_mode::managed;
   };
   
   template<typename T>
   concept form_data_params = {
      requires std::is_base_of_v<default_form_data_params, T>;
   };
   
   template<typename T>
   struct uses_form_data_params : std::false_type {};
   template<template<form_data_params> typename T, form_data_params Params>
   struct uses_form_data_params<T<Params>> : std::true_type {};
   //
   template<typename T>
   concept uses_form_data_params = uses_form_data_params<T>::value;
}

namespace dovah {
   enum class localized_string_category {
      common,
      description,
   };
   
   struct base_managed_form_data {
      public:
         using form_use = managed_form_use;
         // with member functions like `set(base_managed_form_data&, form_stub*)`, etc.
         
         template<localized_string_category Cat>
         using localized_string_use = managed_localized_string_use;
         
         using common_localized_string_use = localized_string_use<localized_string_category::common>;
         using description_localized_string_use = localized_string_use<localized_string_category::description>;
      
      public:
         form_stub& stub;
   };
   struct base_unmanaged_form_data {
      public:
         using form_use = dovah::form_stub*;
         
         template<localized_string_category Cat>
         using localized_string_use = unmanaged_localized_string;
         
         using common_localized_string_use = localized_string_use<localized_string_category::common>;
         using description_localized_string_use = localized_string_use<localized_string_category::description>;
      
      public:
         struct {
            // record flags, etc..
         } stub;
   };
}
namespace dovah::form_data {
   template<form_data_params Params>
   using _base = std::conditional_t<
      Params.management_mode == form_data_management_mode::managef,
      base_managed_form_data,
      base_unmanaged_form_data
   >;
}
namespace dovah {
   template<typename T>
   concept form_data_type = std::is_base_of_v<form_data::_base, T>;
}


namespace dovah::form_data {
   template<form_data_params Params>
   struct shout : public _base {
      struct word_type {
         #define PER_FIELD(X) \
            X(spell) \
            X(word_of_power)
         
         form_use spell;
         form_use word_of_power;
         
         DEFINE_FORM_DATA_VISITORS;
         
         #undef PER_FIELD
      };
      
      common_localized_string_use      name;                // FULL
      description_localized_string_use description;         // DESC
      form_use                         equip_type;          // ETYP -> EQUP
      form_use                         menu_display_object; // MDOB -> STAT
      std::array<word_type, 3>         words;               // SNAM[3]
   };
}


namespace dovah::managed_data_ops {
   template<form_data_type T, uses_form_data_params T>
   void clear_managed_data(form_data_type& form, T& subject) {
      subject.visit([&form](auto& field) {
         using field_type = std::decay_t<decltype(field)>;
         if constexpr (uses_form_data_params<field_type>) {
            clear(form, field);
         } else if constexpr (std::is_base_of_v<managed_form_use, field_type>) {
            field.set(form, nullptr);
         } else if constexpr (std::is_same_v<common_localized_string_use, field_type>) {
            field.clear(form);
         } else if constexpr (std::is_same_v<description_localized_string_use, field_type>) {
            field.clear(form);
         }
      });
   }
   
   template<form_data_type T, uses_form_data_params T>
   void clear_managed_data(form_data_type& form) {
      clear_managed_data(form, form);
   }
   
   
   template<uses_form_data_params A, uses_form_data_params B>
   void assign(A& lhs, B& rhs) {
      constexpr const bool a_managed = std::is_same_v<typename A::form_use, managed_form_use>;
      constexpr const bool b_managed = std::is_same_v<typename B::form_use, managed_form_use>;
      if constexpr (a_managed) {
         if constexpr (b_managed) {
            // managed-to-managed assign
            static_assert(false, "TODO");
         } else {
            // unmanaged-to-managed assign
            static_assert(false, "TODO");
         }
      } else if constexpr (b_managed) {
         // managed-to-unmanaged assign
         static_assert(false, "TODO");
      } else {
         lhs = rhs;
      }
   }
   
}