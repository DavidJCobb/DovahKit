#pragma once
#include <cstdint>
#include <vector>
#include "./form_types.h"
#include "./use_info/entry_flag_underlying_type.h"
#include "./use_info/entry_flag_to_mask.h"

namespace dovah {
   namespace loaded_forms {
      class Form;
   }
   namespace tes_file_reading {
      class subrecord;
   }
   namespace tes_file_writing {
      class subrecord;
   }
   class form_stub;
}

namespace dovah {
   class form_reference_t {
      //
      // Loaded forms should use this class or its subclasses to refer to other forms.
      //
      friend class tes_file_reading::subrecord;
      friend class tes_file_writing::subrecord;
      protected:
         form_stub* stub = nullptr;
         use_info::entry_flag_underlying_type use_info_flags = 0;
         
         explicit form_reference_t(use_info::entry_flag_underlying_type f) : use_info_flags(f) {}
         explicit form_reference_t(use_info::entry_flag_underlying_type f, form_stub* s) : use_info_flags(f), stub(s) {}
         
      public:
         form_reference_t() {}
         form_reference_t(form_stub* s) : stub(s) {}
         
         uint32_t formID() const noexcept;
         constexpr form_stub* get_form_stub() const noexcept { return this->stub; }
         
         void clear_if(loaded_forms::Form& owner, form_stub& clear_if);
         void set(loaded_forms::Form& owner, form_stub* set_to);
         void set(loaded_forms::Form& owner, const form_reference_t& set_to);
         bool form_type_matches(form_type) const noexcept; // always returns (true) if (this->stub == nullptr)
         
         constexpr operator bool() const noexcept { return this->stub != nullptr; }
         constexpr bool operator==(const form_reference_t& other) const noexcept { return this->stub == other.stub; };
         constexpr bool operator!=(const form_reference_t& other) const noexcept { return this->stub != other.stub; };
         constexpr bool operator==(const form_stub* other) const noexcept { return this->stub == other; }
         constexpr bool operator!=(const form_stub* other) const noexcept { return this->stub != other; }
         
         //
         // This function is for internal use only. Hardcoded forms use it during the on-demand 
         // load process, to set up form-to-form references that are present in hardcoded data 
         // (in lieu of having a file to load this data from).
         //
         void unmanaged_set(form_stub* set_to); // FOR INTERNAL USE ONLY
         
      protected:
         inline form_reference_t& operator=(form_stub* other) { this->stub = other; return *this; };
   };

   // 
   // NOTE WHEN USING `unique_form_reference_t`:
   // 
   // Form types' `generate_use_info` functions generally pass the relevant use info 
   // flag manually, since as of this writing there's no way to actually extract it 
   // from the form_reference_t instance. When adding a new use info flag, then, you 
   // will want to edit the referring form's `generate_use_info` function to use the 
   // flag. TODO: Look into templating these eventually and automating that...
   //
   template<auto Flag>
   class unique_form_reference_t : public form_reference_t {
      public:
         static constexpr const auto use_info_flag = Flag;

      public:
         unique_form_reference_t() : form_reference_t(use_info::entry_flag_to_mask(Flag)) {};
         unique_form_reference_t(form_stub* s) : form_reference_t(use_info::entry_flag_to_mask(Flag), s) {};
   };

   extern void clear_form_reference_list(
      std::vector<form_reference_t>&,
      loaded_forms::Form& containing_form_for_list
   );
   extern void remove_form_from_reference_list(
      std::vector<form_reference_t>&,
      form_stub&,
      loaded_forms::Form& containing_form_for_list
   );
   extern void copy_form_reference_list(
      loaded_forms::Form& containing_form_for_destination,
      std::vector<form_reference_t>& destination,
      const std::vector<form_reference_t>& source
   );
}