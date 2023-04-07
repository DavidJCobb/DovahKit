#pragma once
#include "./abstract_input_node.h"

namespace dovahkit::subsystems::worldinput2::binds::nodes {
   class bound_tool final : public abstract_input_node {
      public:
         static constexpr node_type my_type = node_type::bound_tool;
      public:
         bound_tool() : abstract_input_node(my_type) {}

         virtual bool is_valid_parent() const override { return false; }

         void invoke() const;
         void invoke_for_hold_release() const;

      protected:
         virtual node* _clone_impl() const override;
   };
}