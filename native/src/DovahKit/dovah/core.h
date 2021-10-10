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

   struct file_prefix {
      public:
         struct flag {
            flag() = delete;
            enum : uint16_t {
               is_light = 0x1000,
            };
         };
         static constexpr uint16_t undefined = 0xFFFF & ~flag::is_light;
         //
      public:
         uint16_t value_and_flags = undefined;
         //
         file_prefix() {}
         static file_prefix make_light(uint16_t l) {
            file_prefix out;
            out.set_light_prefix(l);
            return out;
         }
         static file_prefix make_heavy(uint8_t l) {
            file_prefix out;
            out.set_load_prefix(l);
            return out;
         }
         static file_prefix from_form_id(bare_form_id_t, bool is_skyrim_classic) noexcept;
         //
         inline bool is_undefined() const noexcept { return this->value_and_flags == undefined; }
         inline bool is_light() const noexcept { return this->value_and_flags & flag::is_light; }
         inline uint8_t load_prefix() const noexcept {
            if (this->is_light())
               return 0xFE;
            return this->value_and_flags & 0xFF;
         }
         inline uint16_t light_prefix() const noexcept {
            if (!this->is_light())
               return 0;
            return this->value_and_flags & 0x0FFF;
         }
         //
         inline void set_load_prefix(uint8_t v) noexcept { this->value_and_flags = v; }
         inline void set_light_prefix(uint16_t v) noexcept { this->value_and_flags = (v & 0x0FFF) | flag::is_light; }
         //
         inline bare_form_id_t min_form_id() const noexcept {
            bare_form_id_t id = (bare_form_id_t)this->load_prefix() << 0x18;
            id |= (bare_form_id_t)this->light_prefix() << 0x0C;
            id |= 0x800;
            return id;
         }
         inline bare_form_id_t max_form_id() const noexcept {
            bare_form_id_t id = this->min_form_id();
            if (this->is_light())
               id |= 0x00000FFF;
            else
               id |= 0x00FFFFFF;
            return id;
         }
         bare_form_id_t coerce_form_id(bare_form_id_t id) const noexcept {
            if (id == 0)
               return id;
            id &= ~0xFF000000;
            if (this->is_light())
               id &= ~0xFFFFF000;
            if (id < 0x800)
               id = 0x800;
            id |= (bare_form_id_t)this->load_prefix()  << 0x18;
            id |= (bare_form_id_t)this->light_prefix() << 0x0C;
            return id;
         }
         inline bool contains_form_id(bare_form_id_t id) const noexcept {
            if (id < this->min_form_id())
               return false;
            if (id > this->max_form_id())
               return false;
            return true;
         }
         inline bare_form_id_t strip_prefix(bare_form_id_t id) const noexcept {
            if (this->is_light())
               return id & 0x00000FFF;
            return id & 0x00FFFFFF;
         }
         //
         inline operator uint16_t() const noexcept { return this->value_and_flags; }
   };

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
         inline form_stub* get_form_stub() const noexcept { return this->stub; }
         //
         void clear_if(loaded_forms::Form& owner, form_stub& clear_if);
         void set(loaded_forms::Form& owner, form_stub* set_to);
         void set(loaded_forms::Form& owner, const form_reference_t& set_to);
         bool form_type_matches(form_type_t) const noexcept; // always returns (true) if (this->stub == nullptr)
         //
         inline operator bool() const noexcept { return this->stub != nullptr; }
         inline bool operator==(const form_reference_t& other) const noexcept { return this->stub == other.stub; };
         inline bool operator!=(const form_reference_t& other) const noexcept { return this->stub != other.stub; };
         inline bool operator==(const form_stub* other) const noexcept { return this->stub == other; }
         inline bool operator!=(const form_stub* other) const noexcept { return this->stub != other; }
         
         //
         // This function is for internal use only. Hardcoded forms use it during the on-demand 
         // load process, to set up form-to-form references that are present in hardcoded data 
         // (in lieu of having a file to load this data from).
         //
         void unmanaged_set(form_stub* set_to); // FOR INTERNAL USE ONLY
         
      protected:
         inline form_reference_t& operator=(form_stub* other) { this->stub = other; return *this; };
   };
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
         form_id_t() {};
         form_id_t(uint32_t i) : value(i) {};
         //
         inline operator uint32_t() const noexcept { return this->value; };
         //
         inline bool operator>(const uint32_t& other) { return this->value > other; };
         inline bool operator<(const uint32_t& other) { return this->value < other; };
         inline bool operator>=(const uint32_t& other) { return this->value >= other; };
         inline bool operator<=(const uint32_t& other) { return this->value <= other; };
         inline bool operator==(const uint32_t& other) { return this->value == other; };
         inline bool operator!=(const uint32_t& other) { return this->value != other; };
         //
         inline bool operator>(const form_id_t& other) { return this->value > other.value; };
         inline bool operator<(const form_id_t& other) { return this->value < other.value; };
         inline bool operator>=(const form_id_t& other) { return this->value >= other.value; };
         inline bool operator<=(const form_id_t& other) { return this->value <= other.value; };
         inline bool operator==(const form_id_t& other) { return this->value == other.value; };
         inline bool operator!=(const form_id_t& other) { return this->value != other.value; };
   };

   namespace loaded_forms {
      class Form;
   }
}