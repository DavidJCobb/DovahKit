#pragma once

namespace dovah::loaded_forms::components::papyrus {}

namespace dovahkit::subsystems::papyrus {
   class known_script;
}

namespace ui::bound_script_models {
   namespace vmad {
      using namespace dovah::loaded_forms::components::papyrus;
   }
   using known_script = dovahkit::subsystems::papyrus::known_script;
}
