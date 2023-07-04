#pragma once
#include <vector>
#include <QString>
#include "helpers/tree/node.h"
#include "./enums/input_device_type.h"

namespace cobb::bitstreams {
   class reader;
   class writer;
}
namespace dovahkit::subsystems::worldinput2 {
   class control_scheme_action;
   class control_scheme_condition;
   class control_scheme_modifier;
}

namespace dovahkit::subsystems::worldinput2 {
   class control_scheme {
      public:
         static constexpr const size_t max_name_length = 1023;

         using node = cobb::node<
            cobb::node_data_with_attributes<control_scheme_action, cobb::node_data_attribute::leaf>,
            control_scheme_condition,
            control_scheme_modifier
         >;

      public:
         control_scheme(input_device_type);
         //
         control_scheme(const control_scheme&);
         control_scheme& operator=(const control_scheme& o);
         //
         control_scheme(control_scheme&&);
         control_scheme& operator=(control_scheme&& o);

         ~control_scheme();

         QString name;
         input_device_type  device_type;
         std::vector<node*> top_level_nodes;

         bool operator==(const control_scheme& other) const;

         void clear();

         static constexpr const uint32_t serialization_data_version = 0;
         //
         // These require a "fresh" bitstream, i.e. you must not have streamed any data 
         // (except the header, automatically, when giving the buffer to the reader). 
         // The tree will use its own serialization version.
         //
         static control_scheme read(cobb::bitstreams::reader&);
         void write(cobb::bitstreams::writer&) const;
   };
}