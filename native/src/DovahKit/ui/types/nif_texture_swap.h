#pragma once
#include <string>

namespace dovah {
   class form_stub;
}

namespace ui::types {
   struct nif_texture_swap {
      std::string       block_name;
      size_t            leaf_index  = 0;
      dovah::form_stub* texture_set = nullptr;
   };
}