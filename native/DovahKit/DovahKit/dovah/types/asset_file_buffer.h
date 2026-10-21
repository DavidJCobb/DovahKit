#pragma once
#include "lu/memory/basic_buffer.h"
#include "lu/memory/buffer_view.h"

namespace dovah {
   // For loose files and for files extracted from a BSA.
   class asset_file_buffer {
      public:
         constexpr asset_file_buffer() = default;
         asset_file_buffer(lu::memory::basic_buffer&& owned);
         asset_file_buffer(lu::memory::buffer_view unowned);

         asset_file_buffer(const asset_file_buffer&) = delete;
         asset_file_buffer& operator=(const asset_file_buffer&) = delete;

         asset_file_buffer(asset_file_buffer&&) noexcept;
         asset_file_buffer& operator=(asset_file_buffer&&) noexcept;

      protected:
         lu::memory::basic_buffer owned;
         lu::memory::buffer_view  view;

      public:
         constexpr const void* data() const noexcept { return this->view.data(); }
         constexpr const void* data_at(size_t at) const noexcept { return this->view.data_at(at); }
         constexpr bool empty() const noexcept { return this->view.empty(); }
         constexpr const void* end() const noexcept { return this->view.end(); }
         constexpr size_t size() const noexcept { return this->view.size(); }

         constexpr bool is_shared() const noexcept {
            return this->owned.empty();
         }

         // Asserts that we are not shared.
         lu::memory::basic_buffer&& take_owned_data();
   };
}
