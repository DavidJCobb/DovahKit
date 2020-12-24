#include "metamethod_names.h"
#include <cstring>

namespace cobb::lua {
   extern bool is_metamethod_name(const char* n) {
      for (auto* mm : metamethod_names)
         if (strcmp(mm, n) == 0)
            return true;
      return false;
   }
}