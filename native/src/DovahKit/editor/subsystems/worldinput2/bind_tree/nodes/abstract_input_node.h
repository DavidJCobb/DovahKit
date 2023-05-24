#pragma once
#include "../node.h"
#include "../../input_sequence.h"

namespace dovahkit::subsystems::worldinput2::binds::nodes {
   class abstract_input_node : public node {
      protected:
         abstract_input_node(node_type t) : node(t) {}

      public:
         static constexpr const size_t max_name_length = 1023;

      public:
         QString name;
         typename input_sequence input_sequence;

         class input_sequence absolute_input_sequence() const;
   };
}