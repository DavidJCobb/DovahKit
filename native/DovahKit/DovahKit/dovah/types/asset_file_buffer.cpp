#include "./asset_file_buffer.h"
#include <cassert>
#include <utility> // std::move

namespace dovah {
   asset_file_buffer::asset_file_buffer(lu::memory::basic_buffer&& owned) {
      this->owned = std::move(owned);
      this->view  = { this->owned.data(), this->owned.size() };
   }
   asset_file_buffer::asset_file_buffer(lu::memory::buffer_view unowned) {
      this->view = unowned;
   }

   asset_file_buffer::asset_file_buffer(asset_file_buffer&& src) noexcept {
      this->owned = std::move(src.owned);
      this->view  = src.view;
      src.view    = {};
   }
   asset_file_buffer& asset_file_buffer::operator=(asset_file_buffer&& src) noexcept {
      this->owned = std::move(src.owned);
      this->view  = src.view;
      src.view    = {};
      return *this;
   }

   lu::memory::basic_buffer&& asset_file_buffer::take_owned_data() {
      assert(!this->is_shared());
      this->view = {};
      return std::move(this->owned);
   }
}