#pragma once
#include <array>
#include <cstdint>
#include <type_traits>
#include "../extra_data.h"
#include "../../../../form_reference_t.h"
#include "../../../../form_types.h"
#include "../../../../use_info/entry_flags/base_extra_data.h"
#include "./common_form.h" // impl::form_type_or_array_thereof and friends

namespace dovah::loaded_forms::components::extra_data_types {
   template<typename Self, uint32_t Signature, auto Allowed, ::dovah::use_info::entry_flags::base_extra_data UseInfoEntryFlag>
      requires impl::form_type_or_array_thereof<Allowed>
   class common_unique_form : public extra_data {
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
         common_unique_form() : extra_data(all_extra_data_types::index_of_type<Self>) {}

      public:
         unique_form_reference_t<UseInfoEntryFlag> form;

      public:
         virtual subrecord_load_result load(tes_file_reading::subrecord&, load_interface_t&) override;
         virtual record_load_result load(tes_file_reading::record&, load_interface_t&) override;
         virtual void save(tes_file_writing::record&, save_interface_t&) override;
         
         virtual void clear_contained_formIDs(loaded_forms::Form& my_owner) override;
         virtual void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) override;
         
         virtual extra_data* clone(loaded_forms::Form& clone_owner) const noexcept override;
   };
}

#include "./common_unique_form.inl"