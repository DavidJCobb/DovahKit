#pragma once
#include <array>
#include <cstdint>
#include <vector>
#include "form_types.h"

namespace dovah {
   class form_stub;
   namespace loaded_forms {
      class Form;
   }
   namespace tes_file_reading {
      class subrecord;
   }
   namespace tes_file_writing {
      class subrecord;
   }

   using bare_form_id_t = uint32_t;

   enum class game {
      skyrim_classic,
      skyrim_special,
   };

   extern bool game_supports_light_plugins(game);

   inline constexpr uint32_t hardcoded_form_id_mask = 0x000007FF; // Mask for form IDs that are hardcoded forms.
   inline constexpr uint32_t plugin_form_id_mask    = 0x00FFF800; // Mask for form IDs that are not hardcoded forms.
   inline constexpr uint32_t minimum_plugin_form_id = 0x00000800; // Minimum non-load-order-prefixed form ID for a non-hardcoded form.
   inline constexpr uint32_t form_id_prefix_mask    = 0xFF000000; // Mask to get the load order prefix of a form ID.

   #pragma region form_reference_t and friends
   class form_reference_t {
      //
      // Loaded forms should use this class or its subclasses to refer to other forms.
      //
      friend class tes_file_reading::subrecord;
      friend class tes_file_writing::subrecord;
      protected:
         form_stub* stub           = nullptr;
         uint8_t    use_info_flags = 0;
         //
         explicit form_reference_t(uint8_t f) : use_info_flags(f) {}
         explicit form_reference_t(uint8_t f, form_stub* s) : use_info_flags(f), stub(s) {}
         //
      public:
         form_reference_t() {}
         form_reference_t(form_stub* s) : stub(s) {}
         //
         bare_form_id_t formID() const noexcept;
         constexpr form_stub* get_form_stub() const noexcept { return this->stub; }
         //
         void clear_if(loaded_forms::Form& owner, form_stub& clear_if);
         void set(loaded_forms::Form& owner, form_stub* set_to);
         void set(loaded_forms::Form& owner, const form_reference_t& set_to);
         bool form_type_matches(form_type_t) const noexcept; // always returns (true) if (this->stub == nullptr)
         //
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
   // Subclasses below automatically manage specific use info flags, for any special 
   // relationships between forms.
   // 
   // NOTE WHEN ADDING NEW SUBCLASSES:
   // 
   // Form types' `generate_use_info` functions generally pass the relevant use info 
   // flag manually, since as of this writing there's no way to actually extract it 
   // from the form_reference_t instance. When adding a new use info flag, then, you 
   // will want to add both a subclass, as below, and to edit the referring form's 
   // `generate_use_info` function to use the flag. TODO: Look into templating these 
   // eventually and automating that...
   //

   class base_form_reference_t : public form_reference_t {
      public:
         base_form_reference_t();
         base_form_reference_t(form_stub* s);
   };
   class dialogue_branch_reference_t : public form_reference_t {
      public:
         dialogue_branch_reference_t();
         dialogue_branch_reference_t(form_stub* s);
   };
   class dialogue_quest_reference_t : public form_reference_t {
      public:
         dialogue_quest_reference_t();
         dialogue_quest_reference_t(form_stub* s);
   };
   class water_acti_type_reference_t : public form_reference_t {
      public:
         water_acti_type_reference_t();
         water_acti_type_reference_t(form_stub* s);
   };

   extern void clear_form_reference_list(std::vector<form_reference_t>&, loaded_forms::Form& owner);
   extern void remove_form_from_reference_list(std::vector<form_reference_t>&, form_stub& target, loaded_forms::Form& owner);
   extern void copy_form_reference_list(loaded_forms::Form& target_owner, std::vector<form_reference_t>& target, const std::vector<form_reference_t>& source);
   #pragma endregion

   struct form_id_t {
      //
      // This struct exists in order to allow the "read"/"write" functions for file I/O to be 
      // templated on form IDs, to automate form ID fixup.
      //
      // Loaded forms should not use this struct as a member. It should only be used for 
      // generating use info, i.e. when you need to load a form ID but not retain it.
      //
      friend class tes_file_reading::subrecord;
      friend class tes_file_writing::subrecord;
      protected:
         uint32_t value = 0;
      public:
         constexpr form_id_t() {};
         constexpr form_id_t(uint32_t i) : value(i) {};
         
         constexpr operator uint32_t() const noexcept { return this->value; };
         
         constexpr bool operator>(const uint32_t& other) { return this->value > other; };
         constexpr bool operator<(const uint32_t& other) { return this->value < other; };
         constexpr bool operator>=(const uint32_t& other) { return this->value >= other; };
         constexpr bool operator<=(const uint32_t& other) { return this->value <= other; };
         constexpr bool operator==(const uint32_t& other) { return this->value == other; };
         constexpr bool operator!=(const uint32_t& other) { return this->value != other; };
         
         constexpr bool operator>(const form_id_t& other) { return this->value > other.value; };
         constexpr bool operator<(const form_id_t& other) { return this->value < other.value; };
         constexpr bool operator>=(const form_id_t& other) { return this->value >= other.value; };
         constexpr bool operator<=(const form_id_t& other) { return this->value <= other.value; };
         constexpr bool operator==(const form_id_t& other) { return this->value == other.value; };
         constexpr bool operator!=(const form_id_t& other) { return this->value != other.value; };
   };

   namespace loaded_forms {
      class Form;
   }
}