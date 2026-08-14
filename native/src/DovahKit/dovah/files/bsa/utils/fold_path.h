#pragma once
#include <string>

namespace dovah::bsa::utils {
   // Case-fold a path, and convert secondary path separators to 
   // primary path separators. Does not detect or merge repeated 
   // directory separators.
   extern void fold_path(std::string&);
}