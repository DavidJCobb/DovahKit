#pragma once
#include <cstdint>
#include "../core.h"

namespace dovah {
   struct file_prefix {
      public:
         struct flag {
            flag() = delete;
            enum : uint16_t {
               is_light = 0x1000,
            };
         };
         static constexpr uint16_t undefined = 0xFFFF & ~flag::is_light;
         
      public:
         uint16_t value_and_flags = undefined;
         
         constexpr file_prefix() {}
         //
         static constexpr file_prefix make_light(uint16_t l);
         static constexpr file_prefix make_heavy(uint8_t l);
         static constexpr file_prefix from_form_id(bare_form_id_t, bool is_skyrim_classic) noexcept;
         
         constexpr bool is_undefined() const noexcept;
         constexpr bool is_light() const noexcept;
         constexpr uint8_t load_prefix() const noexcept;
         constexpr uint16_t light_prefix() const noexcept;
         
         constexpr void set_load_prefix(uint8_t v) noexcept;
         constexpr void set_light_prefix(uint16_t v) noexcept;
         
         constexpr bare_form_id_t min_form_id() const noexcept;
         constexpr bare_form_id_t max_form_id() const noexcept;
         constexpr bare_form_id_t coerce_form_id(bare_form_id_t id) const noexcept; // Remove the form ID's own load order prefix, and force it into this prefix's ID range.
         constexpr bool contains_form_id(bare_form_id_t id) const noexcept;
         constexpr bare_form_id_t strip_prefix(bare_form_id_t id) const noexcept;
         
         constexpr operator uint16_t() const noexcept { return this->value_and_flags; }
   };
}

#include "./file_prefix.inl"