#include "_import_all.h"
#include "all_standard.h"
#include "dovah.h"
#include "dovah_package.h"
#include "form_types.h"
#include "unscoped.h"

namespace dovahscript::lua_libraries {
   extern void import_all(lua_State* L) {
      all_standard::import(L);
      dovah::import(L);
      dovah_package::import(L);
      form_types::import(L);
      unscoped::import(L);
   }
}