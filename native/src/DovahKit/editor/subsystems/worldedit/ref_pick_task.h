#pragma once
#include <functional>
namespace dovah {
   class form_stub;
}

namespace dovahkit::subsystems::worldedit {
   class ref_pick_task {
      public:
         std::function<bool(dovah::form_stub*)> ref_filter;
         struct {
            std::function<void(dovah::form_stub*)> on_complete;
            std::function<void()>                  on_canceled;
         } callbacks;
         bool cancel_on_non_matching_ref = true;
   };
}