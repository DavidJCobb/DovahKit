#pragma once
#include <stdexcept>

namespace dovah {
   class form_stub;
}

namespace dovah::exceptions {
   class actor_base_template_is_cyclical : public std::runtime_error {
      public:
         actor_base_template_is_cyclical(form_stub& started_from, form_stub& seen_twice)
            :
            std::runtime_error("Scanning an ActorBase's template-actor relationships revealed a cyclical reference."),
            started_from(started_from),
            seen_twice(seen_twice)
         {}

         form_stub& started_from;
         form_stub& seen_twice;
   };
}