#pragma once

namespace dovahkit::subsystems::audio {
   // Each of these is given its own submix voice, allowing us to bulk-adjust the 
   // volume of all sounds in a given category.
   enum class sound_category {
      uncategorized,
      dialogue_preview,
   };
}