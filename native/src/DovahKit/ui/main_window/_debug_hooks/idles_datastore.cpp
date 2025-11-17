#include "./idles_datastore.h"
#include "dovah/form_stub.h"
#include "dovah/datastores/idles.h"
#include "editor/core.h"

namespace DovahKitDebug::features {
   /*static*/ void idles_datastore::execute(QWidget* from) {
      dovah::file_load_order* lo = DovahKitCore::get().get_file_load_order();
      if (!lo)
         return;

      dovah::datastores::idles datastore;
      datastore.build(*lo);
      __debugbreak();
   }
}
