#include "./get_all_relevant_template_actors.h"
#include <cstdint>
#include "../exceptions/actor_base_template_is_cyclical.h"
#include "../form_stubs/helpers/get_template_actor.h"

namespace dovah {
   extern relevant_template_actors get_all_relevant_template_actors(dovah::form_stub& subject_actor) {
      using loaded_form_type    = relevant_template_actors::loaded_form_type;
      using loaded_form_pointer = relevant_template_actors::loaded_form_pointer;

      if (!form_stub_helpers::get_template_actor(&subject_actor))
         return {};

      auto loaded = subject_actor.load().ptr_cast<loaded_form_type>();
      if (!loaded)
         return {};

      // Handle the case where A inherits keywords from B, who inherits keywords from C, 
      // who DOESN'T inherit keywords from D, who inherits keywords from E: make sure we 
      // stop inheriting keywords at C.
      uint32_t unbroken_chain = 0xFFFFFFFF;

      relevant_template_actors out;
      {
         form_stub* current = &subject_actor;
         std::vector<form_stub*> seen_actors = { current };
         do {
            auto* tmpl = form_stub_helpers::get_template_actor(current);
            if (!tmpl)
               break;
            if (std::find(seen_actors.begin(), seen_actors.end(), tmpl) != seen_actors.end())
               throw exceptions::actor_base_template_is_cyclical(subject_actor, *tmpl);

            auto loaded_subject = current->load().ptr_cast<loaded_form_type>();
            if (!loaded_subject)
               break;
            
            auto flags = loaded_subject->template_data.flags;
            if (flags == 0) {
               //
               // Even if we reach a point where all template flags are zero, such 
               // that we won't inherit data from this point further, we should still 
               // traverse the template relationships in order to verify that there 
               // isn't a cyclical reference.
               //
               unbroken_chain = 0;
            } else if (unbroken_chain == 0) {
               //
               // Even if we reach a point where all template flags are zero, such 
               // that we won't inherit data from this point further, we should still 
               // traverse the template relationships in order to verify that there 
               // isn't a cyclical reference.
               //
            } else {
               for (size_t i = 0; i < out.templates.all.size(); ++i) {
                  auto&    dst  = out.templates.all[i];
                  uint32_t mask = (1 << i);
                  if (flags & mask) {
                     if (unbroken_chain & mask)
                        dst = tmpl;
                  } else {
                     unbroken_chain &= ~mask;
                  }
               }
            }

            current = tmpl;
            seen_actors.push_back(current);
         } while (true);
      }
      return out;
   }
}
