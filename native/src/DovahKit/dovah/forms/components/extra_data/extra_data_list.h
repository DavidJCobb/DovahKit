#pragma once
#include <array>
#include <cstdint>
#include <string>
#include "../../_common.h"
#include "./types/class_array.h"

namespace dovah {
   namespace loaded_forms::components {
      namespace extra_data_types {
         class extra_data;
      }
      class extra_data_use_info_state;
   }
   class form_stub;
   class form_stub_use_info_builder;
}

namespace dovah::loaded_forms::components {
   using extra_data = extra_data_types::extra_data;

   class extra_data_list {
      public:
         enum class load_result {
            unrecognized,
            failed,
            succeeded,
         };

      protected:
         std::vector<extra_data*> content; // contents are owned. list should only allow one of each type.
         
      public:
         ~extra_data_list();
         
         constexpr const decltype(content)& get_items() const noexcept { return this->content; };
         bool insert(extra_data*); // returns (true) if the insertion succeeded.
         void remove(extra_data*);
         
         load_result load(tes_record_reader&, load_order_interfaces::form_load&);
         void post_load_validation(load_order_interfaces::form_load&);
         void save(tes_record_writer&, load_order_interfaces::form_save&);
         void clear(loaded_forms::Form& my_owner);
         void clone_from(const extra_data_list& source, loaded_forms::Form& owner_of_clone);
         void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept;
         //
         static bool generate_use_info(tes_record_reader&, form_stub_use_info_builder&, extra_data_use_info_state&);
         
      public:
         template<typename T> requires all_extra_data_types::contains_type<T>
         const T* get() const noexcept;

         template<typename T> requires all_extra_data_types::contains_type<T>
         T* get() noexcept;

         template<typename T> requires all_extra_data_types::contains_type<T>
         T* get_or_create() noexcept;

         template<typename T> requires all_extra_data_types::contains_type<T>
         void remove(loaded_forms::Form& my_owner) noexcept;
   };
}

#include "./extra_data_list.inl"