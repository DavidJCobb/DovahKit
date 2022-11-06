#pragma once

namespace dovahkit::subsystems::worldinput {
   // Useful for tools that perform an action at some point, e.g. "press F to 
   // spawn an object at the mouse position" versus "press F to spawn an object 
   // at the on-screen reticle position."
   enum class pointer_position_type {
      mouse,
      reticle,
   };
}
