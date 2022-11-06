#pragma once

namespace dovahkit::subsystems::worldinput {
   enum class selection_operation {
      no_op,
      add,     // select object
      remove,  // deselect object
      toggle,  // toggle selection state of object
      replace, // replace entire selection
   };
}