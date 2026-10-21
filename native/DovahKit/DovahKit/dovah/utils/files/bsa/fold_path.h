#pragma once
#include <string>

namespace dovah::utils::files::bsa {
   // Case-fold a path, and convert secondary path separators to 
   // primary path separators. Does not detect or merge repeated 
   // directory separators.
   extern void fold_path(std::string&);
}