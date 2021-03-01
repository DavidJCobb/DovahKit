#pragma once
#include <array>
#include <vector>
#include "form_stub.h"

namespace dovah {
   class form_stub_use_info_builder {
      friend class form_stub;
      protected:
         struct _pending_entry {
            form_stub* target_stub = nullptr; // form stub
            uint32_t   target_id   = 0;       // form ID
            use_info_entry::flags_t flags = 0;
            //
            _pending_entry() {}
            _pending_entry(uint32_t i, use_info_entry::flags_t f) : target_id(i), flags(f) {}
            _pending_entry(form_stub* s, use_info_entry::flags_t f) : target_stub(s), flags(f) {}
         };
         //
         form_stub& _stub;
         bool _is_active_file    = false;
         bool _is_final_file     = false;
         bool _last_record_flags = 0;
         struct _pending_list {
            //
            // To improve performance, we use a fixed-size array and then fall back to a secondary 
            // vector if we need more storage.
            //
            std::array<_pending_entry, 20> fixed;
            std::vector<_pending_entry>    extra;
            size_t size = 0;
         } _pending;
         //
         static constexpr size_t preallocated_array_size = std::tuple_size<decltype(_pending_list::fixed)>::value;
         //
         form_stub_use_info_builder(form_stub& s);
         //
      public:
         bool is_partial_record = false;
         //
         void add_outbound_reference(form_stub* to_stub, use_info_entry::flags_t flags = 0);
         void add_outbound_reference(uint32_t toFormID, use_info_entry::flags_t flags = 0);
         void cancel_outbound_reference(form_stub* to_stub, use_info_entry::flags_t flags = 0);
         void cancel_outbound_reference(uint32_t toFormID, use_info_entry::flags_t flags = 0);
         void commit();
         
         //
         // This class contains a reference, so it cannot be placed in a vector directly; however, it 
         // can be heap-allocated and the pointers can be stored in a vector. When using this, remember 
         // to delete the builder after you are done with it, whether or not you commit it.
         //
         form_stub_use_info_builder* spawn_subordinate() const noexcept;

         //
         // The same as (spwan_subordinate), but it returns the subordinate builder directly; useful 
         // for if only a fixed number of subordinate builders are needed, as you don't need to delete 
         // them manually, etc..
         //
         form_stub_use_info_builder spawn_subordinate_on_stack() const noexcept;
         
         inline const form_stub* stub() const noexcept { return &this->_stub; }
         inline bool is_active_file() const noexcept { return this->_is_active_file; }
         inline bool is_final_file() const noexcept { return this->_is_final_file; }
         inline uint32_t last_record_flags() const noexcept { return this->_last_record_flags; }
         //
         void clear_pending_use_info() noexcept; // discard any yet-to-be-committed use info inside of this builder instance specifically
         void clear_all_prior_use_info() const noexcept; // delete ALL existing use info for the form stub. needed for TopicInfos due to their bizarre partial-record behavior
         //
         // Generic state information, provided for form types that need it:
         form_id_t extra_form_ids[10];
         void*     extra_pointer = nullptr;
   };
}