#pragma once
#include "../node.h"

namespace dovahkit::subsystems::worldinput2::binds::nodes {
   class root final : public node {
      public:
         static constexpr node_type my_type = node_type::root;
      public:
         root() : node(my_type) {}

         virtual bool is_valid_parent() const override { return true; }

      protected:
         virtual node* _clone_impl() const override { return new root; };
         virtual node* _read_impl(cobb::streams::bitreader&) {};
         virtual node* _write_impl(cobb::streams::bitwriter&) const {};
   };
}
