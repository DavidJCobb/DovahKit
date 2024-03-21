#pragma once

namespace dovah::loaded_forms {
   struct precached_nif_info;
}
namespace nifDK {
   class file;
}

namespace nifDK::utils {
   extern dovah::loaded_forms::precached_nif_info precache_nif_info(const file&);
}