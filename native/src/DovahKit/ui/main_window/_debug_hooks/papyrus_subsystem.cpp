#include "papyrus_subsystem.h"
#include "helpers/performance.h"
#include "editor/subsystems/papyrus/DKPapyrusModel.h"
#include "editor/subsystems/papyrus/core.h"

namespace DovahKitDebug::features {
   /*static*/ void papyrus_subsystem::execute(QWidget* from) {
      #if 1
      auto& subsys = dovahkit::subsystems::papyrus::core::get_or_create();
      
      cobb::benchmark bench;
      bench.begin();
      //
      subsys.index_all_pex_files();
      //
      bench.end();
      auto microseconds = bench.microseconds();
      //
      __debugbreak();
      #else
      auto* model = new DKPapyrusModel();

      cobb::benchmark bench;
      bench.begin();
      //
      model->populate_initial();
      //
      bench.end();
      auto microseconds = bench.microseconds();
      //
      __debugbreak();

      delete model;
      #endif
   }
}
