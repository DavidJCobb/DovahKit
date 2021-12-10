#pragma once
#include "../node.h"

namespace DK3D::binds::nodes {
   class root final : public node {
      public:
         static constexpr node_type my_type = node_type::root;
      public:
         root() : node(my_type) {}

         virtual bool check_still_active(DK3DInputHandler&) const override { return true; }
         virtual bool is_valid_parent() const override { return true; }

      protected:
         virtual node* _clone_impl() const override { return new root; };
   };
}
