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
         virtual void _read_impl(cobb::streams::bitreader&) {};
         virtual void _write_impl(cobb::streams::bitwriter&) const {};
         virtual void _read_impl(cobb::bitstreams::reader&) {};
         virtual void _write_impl(cobb::bitstreams::writer&) const {};

         virtual bool _compare_impl(const node& other) const { return true; };
   };
}
