#include "./idles_datastore.h"
#include "dovah/form_stub.h"
#include "dovah/datastores/idles.h"

// garbage hack to access the LO
#include "editor/core.h"
#include "dovah/files/tes_file_reading/file_loader.h"

namespace DovahKitDebug::features {
   /*static*/ void idles_datastore::execute(QWidget* from) {
      dovah::file_load_order* lo = nullptr;
      {  // garbage hack; we should add a real accessor for this
         auto list = DovahKitCore::get().get_loaded_files();
         if (list.empty())
            return;
         lo = &list[0]->get_load_order();
      }

      dovah::datastores::idles datastore;
      datastore.build(*lo);
      __debugbreak();
   }
}
