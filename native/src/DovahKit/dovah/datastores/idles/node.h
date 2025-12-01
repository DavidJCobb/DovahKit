#pragma once
namespace dovah::datastores {
   class idles;
}

namespace dovah::datastores::impl::idles {
   class node {
      public:
         static constexpr const size_t index_of_none = -1;

         using datastore_type = ::dovah::datastores::idles;

      public:
         constexpr node(datastore_type& d) : datastore(d) {};
         virtual ~node() {}

         datastore_type& datastore;
   };
}
