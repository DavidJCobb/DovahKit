#pragma once

namespace vulkanDK::config {
   //
   // If true, the debug grid will be rendered using alpha transparency (via WBOIT), 
   // and it will use alpha transparency as an improvised approach to anti-aliasing. 
   // If false, the debug grid will be rendered as fully opaque with no built-in 
   // anti-aliasing, instead using alpha testing and fragment shader discards.
   //
   static constexpr bool debug_grid_uses_wboit = true;
}
