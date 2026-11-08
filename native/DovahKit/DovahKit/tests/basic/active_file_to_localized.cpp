#include "./active_file_to_localized.h"
#include "dovah/active_load_order.h"
#include "dovah/form_data/types/s/shout.h"
#include "dovah/form_stub.h"
#include "dovah/load_order_processes/active_file_strings_to_localized/process.h"
#include "dovah/requests/create_form.h"

#include "../pieces/load_skyrim_esm.h"

namespace dovahkit::tests {
   static void on_warning(const dovah::notices::base_warning& warning) {
   }

   extern void active_file_to_localized() {
      auto alo_ptr = pieces::load_skyrim_esm(true);
      if (!alo_ptr)
         return;
      auto& alo = *alo_ptr;

      dovah::form_stub& created_shout = [&alo]() -> dovah::form_stub& {
         dovah::requests::create_form request(alo);
         request.editor_id = "aaaTESTShout01";
         request.form_type = dovah::form_type::shout;
         return request.commit();
      }();

      auto loaded_ptr = created_shout.load_as_type<dovah::form_data::shout>();
      assert(!!loaded_ptr);
      loaded_ptr->name.set_non_localized_string(created_shout, "Test Shout 01");
      loaded_ptr->description.set_non_localized_string(created_shout, "This is a test shout. We're gonna convert it to use l-strings.");
      __debugbreak();

      {
         auto process = dovah::load_order_processes::active_file_strings_to_localized::process({
            .active_load_order = alo,
            .languages         = { dovah::localization::language::english },
         });
         process.execute();
      }

      auto& lsc            = alo.get_localized_string_collection();
      auto* active_strings = lsc.bundle_for_active_file();
      assert(!!active_strings);
      assert(!active_strings->empty());
      assert(loaded_ptr->name.is_localized());
      assert(loaded_ptr->description.is_localized());

      __debugbreak();
   }
}