#include "./get_template_actor.h"
#include "../../form_stub.h"
#include "../../form_types.h"
#include "../../use_info/entry_flags/actor_base.h"
#include "./get_unique_outbound_use.h"

namespace dovah::form_stub_helpers {
   extern form_stub* get_template_actor(const form_stub& subject_actor) {
      if (subject_actor.form_type != form_type::actor_base)
         return nullptr;
      return get_unique_outbound_use<use_info::entry_flags::actor_base::template_actor>(subject_actor);
   }
}