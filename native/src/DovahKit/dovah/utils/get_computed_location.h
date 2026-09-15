#pragma once
namespace dovah {
   namespace loaded_forms {
      class Cell;
      class Worldspace;
   }
   class form_stub;
}

namespace dovah::utils {
   extern form_stub* get_computed_location(const loaded_forms::Cell&);
   extern form_stub* get_computed_location(const loaded_forms::Worldspace&);
   extern form_stub* get_computed_location(const form_stub& stub);
}