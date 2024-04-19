#pragma once
#include <array>
#include <cstdint>
#include <vector>
#include "../core.h"
#include "../form_types.h"

namespace dovah {
   namespace notices {
      class base_form_load_warning;
   }
   namespace tes_file_reading {
      class file_loader;
      class subrecord;
   }
   struct detailed_notice;
   class file_load_order;
   class form_stub;
   class form_reference_t;
}

namespace dovah::load_order_interfaces {
   class form_load {
      friend class form_stub;
      public:
         file_load_order& owner;
         const form_stub& target_stub;
         //
         const tes_file_reading::file_loader* current_file = nullptr;
         //
         bool     is_winning_record = false;
         bool     is_partial_record = false; // you could also check the record flags, but there are certain cases where the flag should be ignored, and this bool better reflects those
         uint32_t last_record_flags = 0;

      public:
         // TIP: This function only logs a warning if it has a warning code. Some helper functions can be 
         // called blindly to create and return warnings that only have a code if there's an actual problem.
         void log_load_warning(const detailed_notice&);

         void log_load_warning(notices::base_form_load_warning&);
         
         #pragma region warn_if_ref_is_wrong_type
            void warn_if_ref_is_wrong_type(form_stub* target, form_type desired, uint32_t subrecord_signature);

            // for conditions:
            void warn_if_ref_is_wrong_type(form_stub* target, std::vector<form_type> desired, uint32_t subrecord_signature);

            template<size_t Size>
            void warn_if_ref_is_wrong_type(form_stub* target, const std::array<form_type, Size>& desired, uint32_t subrecord_signature);

            // Allow all funcs that take `form_stub` to also take `form_reference_t`:
            template<typename Desired>
            void warn_if_ref_is_wrong_type(form_reference_t& target, Desired&& desired, uint32_t sub) {
               warn_if_ref_is_wrong_type(target.get_form_stub(), desired, sub);
            }
         #pragma endregion

         void warn_on_unrecognized_subrecord(const tes_file_reading::subrecord&);

         bool is_active_file() const noexcept;
            
      protected:
         form_load(file_load_order& o, const form_stub& t) : owner(o), target_stub(t) {}
   };
}

#include "./form_load.inl"