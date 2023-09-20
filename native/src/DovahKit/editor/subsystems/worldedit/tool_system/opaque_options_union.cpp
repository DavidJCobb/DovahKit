#include "./opaque_options_union.h"
#include "./options_union.h"

namespace dovahkit::subsystems::worldedit::tools {
   void opaque_options_union::deleter::operator()(opaque_options_union* ou) {
      if (!ou)
         return;
      delete (options_union*)ou;
   }
}