#pragma once
#include <array>
#include <cstdint>
#include <vector>
#include "./data/game.h"
#include "./form_reference_t.h"
#include "./form_types.h"

namespace dovah {
   using bare_form_id_t = uint32_t;

   inline constexpr uint32_t hardcoded_form_id_mask = 0x000007FF; // Mask for form IDs that are hardcoded forms.
   inline constexpr uint32_t plugin_form_id_mask    = 0x00FFF800; // Mask for form IDs that are not hardcoded forms.
   inline constexpr uint32_t minimum_plugin_form_id = 0x00000800; // Minimum non-load-order-prefixed form ID for a non-hardcoded form.
   inline constexpr uint32_t form_id_prefix_mask    = 0xFF000000; // Mask to get the load order prefix of a form ID.

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
}