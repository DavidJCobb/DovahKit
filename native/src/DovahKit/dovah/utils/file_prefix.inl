#pragma once
#include "./file_prefix.h"

namespace dovah {
   /*static*/ constexpr file_prefix file_prefix::make_light(uint16_t l) {
      file_prefix out;
      out.set_light_prefix(l);
      return out;
   }
   /*static*/ constexpr file_prefix file_prefix::make_heavy(uint8_t l) {
      file_prefix out;
      out.set_load_prefix(l);
      return out;
   }
   /*static*/ constexpr file_prefix file_prefix::from_form_id(bare_form_id_t id, bool is_skyrim_classic) noexcept {
      if (!id)
         return file_prefix();
      uint8_t prefix = id >> 0x18;
      if (is_skyrim_classic || prefix != 0xFE)
         return file_prefix::make_heavy(prefix);
      return file_prefix::make_light((id >> 0xC) & 0xFFF);
   }

   constexpr bool file_prefix::is_undefined() const noexcept { return this->value_and_flags == undefined; }
   constexpr bool file_prefix::is_light() const noexcept { return this->value_and_flags & flag::is_light; }
   constexpr uint8_t file_prefix::load_prefix() const noexcept {
      if (this->is_light())
         return 0xFE;
      return this->value_and_flags & 0xFF;
   }
   constexpr uint16_t file_prefix::light_prefix() const noexcept {
      if (!this->is_light())
         return 0;
      return this->value_and_flags & 0x0FFF;
   }

   constexpr void file_prefix::set_load_prefix(uint8_t v) noexcept {
      this->value_and_flags = v;
   }
   constexpr void file_prefix::set_light_prefix(uint16_t v) noexcept {
      this->value_and_flags = (v & 0x0FFF) | flag::is_light;
   }

   constexpr bare_form_id_t file_prefix::min_form_id() const noexcept {
      bare_form_id_t id = (bare_form_id_t)this->load_prefix() << 0x18;
      id |= (bare_form_id_t)this->light_prefix() << 0x0C;
      id |= 0x800;
      return id;
   }
   constexpr bare_form_id_t file_prefix::max_form_id() const noexcept {
      bare_form_id_t id = this->min_form_id();
      if (this->is_light())
         id |= 0x00000FFF;
      else
         id |= 0x00FFFFFF;
      return id;
   }
   constexpr bare_form_id_t file_prefix::coerce_form_id(bare_form_id_t id) const noexcept {
      if (id == 0)
         return id;
      id &= ~0xFF000000;
      if (this->is_light())
         id &= ~0xFFFFF000;
      if (id < 0x800)
         id = 0x800;
      id |= (bare_form_id_t)this->load_prefix() << 0x18;
      id |= (bare_form_id_t)this->light_prefix() << 0x0C;
      return id;
   }
   constexpr bool file_prefix::contains_form_id(bare_form_id_t id) const noexcept {
      if (id < this->min_form_id())
         return false;
      if (id > this->max_form_id())
         return false;
      return true;
   }
   constexpr bare_form_id_t file_prefix::strip_prefix(bare_form_id_t id) const noexcept {
      if (this->is_light())
         return id & 0x00000FFF;
      return id & 0x00FFFFFF;
   }


}