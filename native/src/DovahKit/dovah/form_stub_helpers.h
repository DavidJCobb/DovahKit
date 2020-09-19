#pragma once
#include <functional>

namespace dovah {
   class form_stub;
   //
   namespace form_stub_helpers {
      void for_each_child_form(const form_stub* parent, std::function<bool(form_stub*)> functor); // return (true) to stop looping early
      form_stub* get_worldspace_persistent_cell(const form_stub*);
   }
}