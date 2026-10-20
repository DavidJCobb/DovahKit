#pragma once
#include <optional>
namespace dovah {
   class form_stub;
}

namespace dovah {
   class form_stub_addenda {
      public:
         form_stub* persistent_cell     = nullptr; // WRLD persistent cell
         form_stub* canonical_landscape = nullptr; // last loaded LAND for a CELL
   };
}
