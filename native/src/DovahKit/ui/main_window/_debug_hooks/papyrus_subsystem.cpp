#include "papyrus_subsystem.h"
#include "helpers/performance.h"
#include "editor/subsystems/papyrus/core.h"

namespace DovahKitDebug::features {
   /*static*/ void papyrus_subsystem::execute(QWidget* from) {
      auto& subsys = dovahkit::subsystems::papyrus::core::get_or_create();
      
      cobb::benchmark bench;
      bench.begin();
      //
      //subsys.index_all_pex_files(); // lol this is a protected member function now
      //
      bench.end();
      auto microseconds = bench.microseconds();
      //
      __debugbreak();
   }
}
