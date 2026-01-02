#pragma once
#include <glm/glm.hpp>
#include "./_base.h"

namespace dovah {
   class form_stub;
}

namespace dovahkit::subsystems::worldedit::tools {
   class attempt_on_screen_pick_ref : public _base {
      public:
         static constexpr const char*          function_name = "attempt_on_screen_pick_ref";
         static constexpr const cobb::eight_cc function_code = "PikRefAt";

         static constexpr const compile_time_tool_options compile_time_options = {
            .use_strict_ordering = false,
         };
         static constexpr const bool is_raycast_sensitive = false;

      public:
         // The current design of how Worldinput queues tools for execution is such 
         // that the tool has to store a response, even if it's an empty one.
         struct response {};

      public:
         static void request(const tool_request_cause&, const options_union&, tool_response_tuple&);
         static void request_for_hold_release(const options_union&, tool_response_tuple&);

         static void invoke(const response&);
   };
}

#include "./attempt_on_screen_pick_ref.inl"