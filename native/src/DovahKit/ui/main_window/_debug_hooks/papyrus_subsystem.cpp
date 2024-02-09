#include "papyrus_subsystem.h"
#include "helpers/performance.h"
#include "editor/subsystems/papyrus/DKPapyrusModel.h"

namespace DovahKitDebug::features {
   /*static*/ void papyrus_subsystem::execute(QWidget* from) {
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
   }
}
