#pragma once
#include <QString>
#include "../enums/input_device_type.h"

namespace cobb::bitstreams {
   class reader;
   class writer;
}
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
         static constexpr const size_t max_name_length = 1023;

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

         // These require a "fresh" bitstream, i.e. you must not have streamed any data 
         // (except the header, automatically, when giving the buffer to the reader). 
         // The tree will use its own serialization version. (TODO: Do we want to have 
         // separate serialization versions for the data versus the stream internals?)
         static tree read(cobb::bitstreams::reader&);
         void write(cobb::bitstreams::writer&) const;
   };
}