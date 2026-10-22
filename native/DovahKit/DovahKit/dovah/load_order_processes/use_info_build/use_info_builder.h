#pragma once
#include <array>
#include <cstdint>
#include "lu/containers/small_vector.h"
#include "dovah/types/bare_form_id.h"
#include "dovah/use_info/entry_flag_to_mask.h"
#include "dovah/use_info/entry_flag_underlying_type.h"
#include "dovah/use_info/is_entry_flag_type.h"

namespace dovah {
   namespace load_order_processes::use_info_build {
      class use_info_builder_form_specific_data;
      class worker_thread;
   }
   class form_stub;
}

namespace dovah::load_order_processes::use_info_build {
   class use_info_builder {
      friend worker_thread;
      friend form_stub;
      public:
         using form_specific_data = use_info_builder_form_specific_data;

      protected:
         struct pending_entry {
            form_stub*   target_stub = nullptr; // form stub
            bare_form_id target_id   = 0;       // form ID
            use_info::entry_flag_underlying_type flags = 0;
         };
         
         form_stub& _stub;
         bool _is_active_file    = false;
         bool _is_base_record    = false;
         bool _is_final_file     = false;
         bool _is_partial_record = false;
         lu::small_vector<pending_entry, 20> _pending;
         form_specific_data* _form_specific_data = nullptr;
         use_info_builder*   _parent = nullptr;
         
      protected:
         use_info_builder(form_stub& s);
      public:
         ~use_info_builder();
         
      public:
         void add_outbound_reference(bare_form_id toFormID, use_info::entry_flag_underlying_type flags = 0);
         void cancel_outbound_reference(bare_form_id toFormID, use_info::entry_flag_underlying_type flags = 0);

         template<typename UseInfoFlag> requires use_info::is_entry_flag_type_v<UseInfoFlag>
         void add_outbound_reference(bare_form_id to_form_id, UseInfoFlag flag) {
            this->add_outbound_reference(to_form_id, use_info::entry_flag_to_mask(flag));
         }
         template<typename UseInfoFlag> requires use_info::is_entry_flag_type_v<UseInfoFlag>
         void cancel_outbound_reference(bare_form_id to_form_id, UseInfoFlag flag) {
            this->cancel_outbound_reference(to_form_id, use_info::entry_flag_to_mask(flag));
         }

         void commit();
         
         //
         // This class contains a reference, so it cannot be placed in a vector directly; however, it 
         // can be heap-allocated and the pointers can be stored in a vector. When using this, remember 
         // to delete the builder after you are done with it, whether or not you commit it.
         //
         [[nodiscard]] use_info_builder* spawn_subordinate() noexcept;

         //
         // The same as (spwan_subordinate), but it returns the subordinate builder directly; useful 
         // for if only a fixed number of subordinate builders are needed, as you don't need to delete 
         // them manually, etc..
         //
         [[nodiscard]] use_info_builder spawn_subordinate_on_stack() noexcept;
         
         constexpr const form_stub& stub() const noexcept { return this->_stub; }
         constexpr bool is_active_file() const noexcept { return this->_is_active_file; }
         constexpr bool is_base_record() const noexcept { return this->_is_base_record; }
         constexpr bool is_final_file() const noexcept { return this->_is_final_file; }
         constexpr bool is_partial_record() const noexcept { return this->_is_partial_record; }
         constexpr uint32_t last_record_flags() const noexcept { return this->_last_record_flags; }
         
         void clear_pending_use_info() noexcept; // discard any yet-to-be-committed use info inside of this builder instance specifically
         void clear_all_prior_use_info() const noexcept; // delete ALL existing use info for the form stub. needed for TopicInfos due to their bizarre partial-record behavior

         form_specific_data* get_form_specific_data();
         constexpr form_specific_data* get_form_specific_data_if_exists() const noexcept { return this->_form_specific_data; }
         
         //
         // Generic state information for form types that need it (generally to handle use info that 
         // gets generated from data being coalesced across multiple/all records that define a form). 
         // This is used as an alternative to `form_specific_data` for form types that occur very 
         // frequently and may coalesce lots of data. (For example, without this field, every single 
         // Topic form would need to create an FSD; those get allocated on the heap; it'd bog down 
         // use info generation.)
         //
         std::array<bare_form_id, 10> extra_form_ids = {};
   };
}