#include "./bsa_archived_file.h"

namespace dovah {
   cobb::generic_buffer&& bsa_archived_file::take_owned_data() noexcept {
      assert(!is_shared());
      return std::move(this->owned);
   }
}