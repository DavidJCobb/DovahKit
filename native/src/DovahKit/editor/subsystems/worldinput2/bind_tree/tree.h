#pragma once
#include <QString>
#include "../enums/input_device_type.h"

namespace cobb::streams {
   class bitreader;
   class bitwriter;
}
namespace dovahkit::subsystems::worldinput2 {
   namespace binds {
      namespace nodes {
         class root;
      }
   }
}

namespace dovahkit::subsystems::worldinput2::binds {
   class tree {
      public:
         tree(input_device_type);
         //
         tree(const tree&);
         tree& operator=(const tree& o);
         //
         tree(tree&&);
         tree& operator=(tree&& o);

         ~tree();

         // -----

         QString name;
         input_device_type device_type;
         nodes::root*      root   = nullptr;

         bool operator==(const tree& other) const;

         static constexpr const uint32_t serialization_version = 0;
         //
         static tree read(cobb::streams::bitreader&);
         void write(cobb::streams::bitwriter&) const;
   };
}