#pragma once
#include "../node.h"

namespace dovahkit::subsystems::worldinput::binds::nodes {
   class root final : public node {
      public:
         static constexpr node_type my_type = node_type::root;
      public:
         root() : node(my_type) {}

         virtual bool check_still_active(core&) const override { return true; }
         virtual bool is_valid_parent() const override { return true; }

      protected:
         virtual node* _clone_impl() const override { return new root; };
   };
}
