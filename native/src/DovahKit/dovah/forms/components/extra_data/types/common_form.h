#pragma once
#include <array>
#include <cstdint>
#include <type_traits>
#include "../extra_data.h"
#include "../../../../form_reference_t.h"
#include "../../../../form_types.h"

namespace dovah::loaded_forms::components::extra_data_types {
   namespace impl {
      template<typename T>
      concept type_is_form_type_or_array_thereof = std::is_same_v<T, form_type> || requires {
         typename T::value_type;
         requires std::is_same_v<T, std::array<typename T::value_type, std::tuple_size_v<T>>>;
      };

      template<auto Value>
      concept form_type_or_array_thereof = type_is_form_type_or_array_thereof<decltype(Value)>;
   }

   template<typename Self, uint32_t Signature, auto Allowed> requires impl::form_type_or_array_thereof<Allowed>
   class common_form : public extra_data {
      public:
         static constexpr const uint32_t signature = Signature;

         static constexpr const auto allowed_form_types = []() {
            using type = decltype(Allowed);
            if constexpr (std::is_same_v<type, form_type>) {
               if constexpr (Allowed == form_type::none) {
                  return std::array<form_type, 0>{};
               } else {
                  return std::array{ Allowed };
               }
            } else {
               return Allowed;
            }
         }();

      public:
         common_form() : extra_data(all_extra_data_types::index_of_type<Self>) {}

      public:
         form_reference_t form;

      public:
         virtual subrecord_load_result load(tes_file_reading::subrecord&, load_interface_t&) override;
         virtual record_load_result load(tes_file_reading::record&, load_interface_t&) override;
         virtual void save(tes_file_writing::record&, save_interface_t&) override;
         
         virtual void clear_contained_formIDs(loaded_forms::Form& my_owner) override;
         virtual void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) override;
         
         virtual extra_data* clone(loaded_forms::Form& clone_owner) const noexcept override;
   };
}

#include "./common_form.inl"