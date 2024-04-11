#pragma once
#include <string>
#include <utility>
#include <vector>

namespace nifDK {
   class file;
}

namespace nifDK::utils {
   // returns a list of NiGeometry with remappable textures, as leaf-index-and-block-name pairs
   extern std::vector<std::pair<size_t, std::string>> find_all_retexturable_blocks(const file&);
}