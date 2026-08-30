#include "./full_load_every_form.h"
#include "dovah/form_stub.h"
#include "editor/core.h"

namespace DovahKitDebug::features::mega_tests {
   /*static*/ void full_load_every_form::execute(QWidget* from) {
      auto& editor = DovahKitCore::get();
      editor.for_each_form([](dovah::form_stub* form) -> bool {
         if (form->is_none_stub())
            return false;
         if (form->form_type == dovah::form_type::none) // PapyrusPersistenceForm, etc.; these have no data that can be loaded
            return false;
         if (form->form_type == dovah::form_type::setting) // not actually handled as a "form" with "data" that we can "load" as such
            return false;

         auto loaded = form->load();
         if (!loaded)
            __debugbreak();

         return false;
      });
   }
}
