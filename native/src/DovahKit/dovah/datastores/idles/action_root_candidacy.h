#pragma once
namespace dovah::datastores::impl::idles {
   class idle_node;
}
namespace dovah::tes_file_reading {
   class file_loader;
}

namespace dovah::datastores::impl::idles {
   struct action_root_candidacy {
      const tes_file_reading::file_loader* source_file = nullptr;
      struct {
         size_t of_record    = 0; // record offset within containing file
         size_t of_subrecord = 0; // subrecord offset within record body (in case record was compressed and had multiple ANAM))
      } offsets;
   };
}