#include "papyrus_subsystem.h"
#include "editor/subsystems/papyrus/DKPapyrusModel.h"

namespace DovahKitDebug::features {
   /*static*/ void papyrus_subsystem::execute(QWidget* from) {
      auto* model = new DKPapyrusModel();
      model->populate_initial();
      __debugbreak();
      delete model;
   }
}
