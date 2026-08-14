#pragma once

namespace dovah::bsa {
   // Path separator encoded into BSA files.
   constexpr const char preferred_path_separator = '\\';

   // Secondary path separator; game engine will normalize this to the preferred 
   // path separator.
   constexpr const char secondary_path_separator = '/';
}