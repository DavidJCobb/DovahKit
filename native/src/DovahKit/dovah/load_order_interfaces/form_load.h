#pragma once
#include <array>
#include <cstdint>
#include <vector>
#include "../form_reference_t.h"
#include "../form_types.h"
#include "../notices/form_load_warnings/form_reference_type_mismatch.h"

namespace dovah {
   namespace notices {
      class base_form_load_warning;
   }
   namespace tes_file_reading {
      class file_loader;
      class subrecord;
   }
   class file_load_order;
   class form_stub;
   class form_reference_t;
}

namespace dovah::load_order_interfaces {
   class form_load {
      friend class form_stub;
      public:
         file_load_order& owner;
         form_stub&       target_stub;
         //
         const tes_file_reading::file_loader* current_file = nullptr;
         //
         bool     is_winning_record = false;
         bool     is_partial_record = false; // you could also check the record flags, but there are certain cases where the flag should be ignored, and this bool better reflects those
         uint32_t last_record_flags = 0;

      protected:
         // Is this load occurring during the cleanup phase of a file-save operation? If 
         // so, we shouldn't log anything. The main case here is during the cleanup phase 
         // of a file-save operation, wherein form data is loaded so that we can delete 
         // unsaved forms from memory. We want to avoid actually logging any warnings 
         // because those warnings may include pointers to the to-be-deleted form, which 
         // will be left dangling.
         bool is_during_file_save_cleanup = false;

      public:
         void log_load_warning(notices::base_form_load_warning&);
         
         #pragma region warn_if_ref_is_wrong_type
            void warn_if_ref_is_wrong_type(form_stub* target, form_type desired, uint32_t subrecord_signature);

            void warn_if_ref_is_wrong_type(
               form_stub* target,
               form_type  desired,
               const tes_file_reading::subrecord& subrecord,
               const notices::form_load_warnings::form_reference_type_mismatch::metadata_type& metadata
            );
            void warn_if_ref_is_wrong_type(
               const form_reference_t& target,
               form_type               desired,
               const tes_file_reading::subrecord& subrecord,
               const notices::form_load_warnings::form_reference_type_mismatch::metadata_type& metadata
            );

            // for conditions:
            void warn_if_ref_is_wrong_type(form_stub* target, std::vector<form_type> desired, uint32_t subrecord_signature);

            template<size_t Size>
            void warn_if_ref_is_wrong_type(form_stub* target, const std::array<form_type, Size>& desired, uint32_t subrecord_signature);

            // Allow all funcs that take `form_stub` to also take `form_reference_t`:
            template<typename... Args>
            void warn_if_ref_is_wrong_type(form_reference_t& target, Args&&... args) {
               warn_if_ref_is_wrong_type(target.get_form_stub(), std::forward<Args>(args)...);
            }
         #pragma endregion

         void warn_on_unrecognized_subrecord(const tes_file_reading::subrecord&);

         bool is_active_file() const noexcept;
            
      protected:
         form_load(file_load_order& o, form_stub& t) : owner(o), target_stub(t) {}
   };
}

#include "./form_load.inl"