#pragma once
#include "./form_stub.h"

namespace dovah {
   #pragma region Source file member functions
   constexpr form_stub::file_data* form_stub::_get_source_file_info(int16_t i) noexcept {
      return const_cast<file_data*>(std::as_const(*this).get_source_file_info(i));
   }
   constexpr const form_stub::file_data* form_stub::get_source_file_info(int16_t i) const noexcept {
      if (!this->has_multiple_source_files()) {
         if (i != 0 && i != -1)
            return nullptr;
         return const_cast<file_data*>(&this->file);
      }
      if (i < 0)
         i += this->files.count;
      if (i >= this->files.count)
         return nullptr;
      return &this->files.entries[i];
   }

   constexpr bool form_stub::file_list_includes(const owner_file_t* f) const noexcept {
      if (!this->has_multiple_source_files())
         return this->file.pointer == f;
      auto size = this->files.count;
      for (uint16_t i = 0; i < size; ++i)
         if (this->files.entries[i].pointer == f)
            return true;
      return false;
   }
   constexpr bool form_stub::has_source_files() const noexcept {
      if (!this->has_multiple_source_files())
         return this->file;
      return this->files.entries != nullptr;
   }
   constexpr bool form_stub::has_multiple_source_files() const noexcept {
      return this->flags & flag::has_multiple_source_files;
   }
   constexpr int16_t form_stub::source_file_count() const noexcept {
      if (!this->has_multiple_source_files()) {
         if (!this->file)
            return 0;
         return 1;
      }
      return this->files.count;
   }
   constexpr int16_t form_stub::index_of_file(const owner_file_t* f) const noexcept {
      if (!this->has_multiple_source_files()) {
         if (this->file.pointer == f)
            return 0;
         return -1;
      }
      auto size = this->files.count;
      for (uint16_t i = 0; i < size; ++i)
         if (this->files.entries[i].pointer == f)
            return i;
      return -1;
   }
   constexpr form_stub::owner_file_t* form_stub::get_file_at_index(int16_t i) const noexcept {
      auto* data = this->get_source_file_info(i);
      if (data)
         return data->pointer;
      return nullptr;
   }
   constexpr uint32_t form_stub::get_file_offset(int16_t file_index) const noexcept {
      auto* data = this->get_source_file_info(file_index);
      if (data)
         return data->offset;
      return 0;
   }
   #pragma endregion

   constexpr bool form_stub::is_deleted() const noexcept {
      return this->test_record_flags(tes_file_record_header::flag::deleted);
   };
   constexpr bool form_stub::is_edited() const noexcept {
      return (bool)(this->flags & flag::is_edited);
   }
   constexpr bool form_stub::is_hardcoded() const noexcept {
      return (bool)(this->flags & flag::is_hardcoded);
   }

   #pragma region Record flag functions
   constexpr uint32_t form_stub::get_record_flags() const noexcept {
      auto* info = this->get_source_file_info();
      if (!info)
         return 0;
      return info->flags;
   }
   constexpr bool form_stub::test_record_flags(uint32_t mask) const noexcept {
      return this->test_record_flags_for_file(mask, -1);
   }
   constexpr bool form_stub::test_record_flags_for_file(uint32_t mask, int16_t file_index) const noexcept {
      auto* info = this->get_source_file_info(file_index);
      if (!info)
         return false;
      return (info->flags & mask) == mask;
   }
   constexpr bool form_stub::test_record_flags_for_file(uint32_t mask, owner_file_t& f) const noexcept {
      return this->test_record_flags_for_file(mask, this->index_of_file(&f));
   }
   #pragma endregion
}