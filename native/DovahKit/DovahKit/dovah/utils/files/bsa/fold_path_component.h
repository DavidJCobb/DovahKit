#pragma once
#include <string>

namespace dovah::utils::files::bsa {
   // Case-fold a path component.
   extern void fold_path_component(std::string&);
}