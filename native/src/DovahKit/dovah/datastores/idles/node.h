#pragma once

namespace dovah::datastores::impl::idles {
   class node {
      public:
         static constexpr const size_t no_index = -1;

      public:
         virtual ~node() {}
   };
}
