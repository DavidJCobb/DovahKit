#pragma once
namespace dovah::datastores {
   class idles2;
}

namespace dovah::datastores::impl::idles2 {
   class node {
      public:
         static constexpr const size_t index_of_none = -1;

         using datastore_type = ::dovah::datastores::idles2;

      public:
         constexpr node(datastore_type& d) : datastore(d) {};
         virtual ~node() {}

         datastore_type& datastore;
   };
}
