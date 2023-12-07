#pragma once
#include <string>
#include "./inheritance_status.h"
#include "./property_value.h"

namespace dovah::papyrus {
   class property {
      public:
         std::string    name;
         property_value value;
         //
         inheritance_status inheritance;
   };
}