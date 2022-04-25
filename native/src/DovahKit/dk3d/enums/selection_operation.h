#pragma once

namespace DK3D {
   enum class selection_operation {
      no_op,
      add,     // select object
      remove,  // deselect object
      toggle,  // toggle selection state of object
      replace, // replace entire selection
   };
}