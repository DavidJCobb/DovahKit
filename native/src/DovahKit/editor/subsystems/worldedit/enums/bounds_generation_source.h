#pragma once

namespace dovahkit::subsystems::worldedit {
   enum class bounds_generation_source {
      undefined,
      nif,  // We generated bounds information using the NIF's vertices.
      obnd, // We generated bounds information using the base form's OBND.
      none, // We were unable to generate bounds information and fell back to a default size.
   };
}