#pragma once
#include "../form_stub.h"
#include "../forms/ActorBase.h"

namespace dovah {
   //
   // Given a subject actor, this struct will tell you what template actor it inherits 
   // from for each template flag. Template actor relationships can be transitive, and 
   // these transitive "chains" of templating may stop at different "links" depending 
   // on what flags each "link" has set.
   //
   struct relevant_template_actors {
      public:
         using loaded_form_type    = loaded_forms::ActorBase;
         using loaded_form_pointer = loaded_form_ptr<loaded_form_type>;

      public:
         loaded_form_pointer subject;
         union _ {
            _() {}
            _(const _& o) : all(o.all) {}
            ~_() {
               this->all.~array();
            }

            struct {
               loaded_form_pointer traits;
               loaded_form_pointer stats;
               loaded_form_pointer factions;
               loaded_form_pointer magic;
               loaded_form_pointer ai_data;
               loaded_form_pointer ai_packages;
               loaded_form_pointer animations;
               loaded_form_pointer base_data;
               loaded_form_pointer inventory;
               loaded_form_pointer scripts;
               loaded_form_pointer ai_package_formlists;
               loaded_form_pointer attack_data;
               loaded_form_pointer keywords;
            };
            std::array<loaded_form_pointer, 13> all = {};
         } templates;
   };

   extern relevant_template_actors get_all_relevant_template_actors(form_stub& subject_actor);
}
