#pragma once
#include "./abstract_input_node.h"
#include "../node.h"
#include "../../input_sequence.h"

namespace dovahkit::subsystems::worldinput2::binds::nodes {
   class modifier : public abstract_input_node {
      public:
         static constexpr node_type my_type = node_type::modifier;
      public:
         modifier() : abstract_input_node(my_type) {}

         virtual bool is_valid_parent() const { return true; }

      protected:
         virtual node* _clone_impl() const override;
   };
}