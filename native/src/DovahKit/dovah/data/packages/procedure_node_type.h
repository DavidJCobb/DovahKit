#pragma once

namespace dovah::packages {
   enum class procedure_node_type {
      procedure, // "Procedure"

      random,       // "Random"
      sequence,     // "Sequence"
      simultaneous, // "Simultaneous"
      stacked,      // "Stacked"
   };
}