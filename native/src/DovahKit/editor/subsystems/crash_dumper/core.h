#pragma once
#include "helpers/singleton_ex.h"

namespace dovahkit::subsystems::crash_dumper {
   class core : public cobb::singleton_ex<core> {
      protected:
         core();
         ~core();

      public:
         void register_new_thread();
   };
}