#pragma once
#include "./form_stub.h"
#include "lu/foreach_lambda_result.h"
#include "./tes/record_flags.h"

namespace dovah {
   constexpr bool form_stub::is_deleted() const noexcept {
      return this->test_record_flags((uint32_t)1 << (uint32_t)tes::record_flag::deleted);
   };
   constexpr bool form_stub::is_edited() const noexcept {
      return this->flags.has_all_of<flag::is_edited>();
   }
   constexpr bool form_stub::is_hardcoded() const noexcept {
      return this->flags.has_all_of<flag::is_hardcoded>();
   }

   #pragma region Record flag functions
      constexpr uint32_t form_stub::get_record_flags() const noexcept {
         auto* info = this->get_source_file_info();
         if (!info)
            return 0;
         return info->record_flags;
      }
      constexpr bool form_stub::test_record_flags(uint32_t mask) const noexcept {
         return this->test_record_flags_for_file(mask, -1);
      }
      constexpr bool form_stub::test_record_flags_for_file(uint32_t mask, int16_t file_index) const noexcept {
         auto* info = this->get_source_file_info(file_index);
         if (!info)
            return false;
         return (info->record_flags & mask) == mask;
      }
      constexpr bool form_stub::test_record_flags_for_file(uint32_t mask, const tes::file& f) const noexcept {
         const auto i = this->index_of_file(f);
         if (i < 0)
            return false;
         return this->test_record_flags_for_file(mask, i);
      }
   #pragma endregion
   #pragma region Source file member functions
      constexpr bool form_stub::file_list_includes(const tes::file& f) const noexcept {
         if (!this->has_multiple_source_files())
            return this->file.pointer == &f;
         auto size = this->files.count;
         for (uint16_t i = 0; i < size; ++i)
            if (this->files.entries[i].pointer == &f)
               return true;
         return false;
      }
      constexpr tes::file* form_stub::get_file_at_index(int16_t i) const noexcept {
         auto* data = this->get_source_file_info(i);
         if (data)
            return data->pointer;
         return nullptr;
      }
      constexpr uint32_t form_stub::get_record_offset(int16_t file_index) const noexcept {
         auto* data = this->get_source_file_info(file_index);
         if (data)
            return data->record_offset;
         return 0;
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
      constexpr bool form_stub::has_source_files() const noexcept {
         if (!this->has_multiple_source_files())
            return this->file;
         return this->files.entries != nullptr;
      }
      constexpr bool form_stub::has_multiple_source_files() const noexcept {
         return this->flags.has_all_of<flag::has_multiple_source_files>();
      }
      constexpr int16_t form_stub::index_of_file(const tes::file& f) const noexcept {
         if (!this->has_multiple_source_files()) {
            if (this->file.pointer == &f)
               return 0;
            return -1;
         }
         auto size = this->files.count;
         for (uint16_t i = 0; i < size; ++i)
            if (this->files.entries[i].pointer == &f)
               return i;
         return -1;
      }
      constexpr int16_t form_stub::source_file_count() const noexcept {
         if (!this->has_multiple_source_files()) {
            if (!this->file)
               return 0;
            return 1;
         }
         return this->files.count;
      }
   #pragma endregion

   template<typename Callable, std::same_as<tes::file_view> FileView>
      requires requires(Callable&& callable, tes::record_view& record, load_order_interfaces::form_load& intfc) {
         { callable(record, intfc) };
      }
   void form_stub::for_each_source_record(Callable&& callable) {
      constexpr const bool can_stop_early =
         // if `Callable` has a non-static member `operator()`:
         std::is_invocable_r_v<lu::foreach_lambda_result, Callable, Callable&, tes::record_view&, load_order_interfaces::form_load&> ||
         // if `Callable` is a free function:
         std::is_invocable_r_v<lu::foreach_lambda_result, Callable, tes::record_view&, load_order_interfaces::form_load&>
      ;

      load_order_interfaces::form_load intfc{ *this };

      if (!this->has_multiple_source_files()) {
         intfc.is_winning_record = true;
         if (this->file.record_offset != 0) {
            FileView view = _open_record(*this->file.pointer, this->file.record_offset);
            callable(view.subviews.record, intfc);
         }
         return;
      }
      for (uint16_t i = 0; i < this->files.count; ++i) {
         const auto& info = this->files.entries[i];
         if (info.record_offset == 0) // dummy file
            continue;
         intfc.is_winning_record = i == this->files.count - 1;
         intfc.is_partial_record = info.record_flags & ((uint32_t)1 << (size_t)tes::record_flag::partial);
         if (i == 0) {
            intfc.is_partial_record = false;
         } else {
            intfc.last_record_flags = this->files.entries[i - 1].record_flags;
         }
         FileView file_view = _open_record(*info.pointer, info.record_offset);
         if constexpr (can_stop_early) {
            auto result = callable(file_view.subviews.record, intfc);
            if (result == lu::foreach_lambda_result::stop)
               break;
         } else {
            callable(file_view.subviews.record, intfc);
         }
      }
   }
}