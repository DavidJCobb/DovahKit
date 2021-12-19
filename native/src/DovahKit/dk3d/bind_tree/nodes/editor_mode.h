#pragma once
#include "../node.h"
#include "../../enums/editor_mode.h"

namespace DK3D::binds::nodes {
   class editor_mode final : public node {
      public:
         static constexpr node_type my_type = node_type::editor_mode;
         using value_type = DK3D::editor_mode;
      public:
         editor_mode(value_type m) : node(my_type), mode(m) {}

         value_type mode;

         virtual bool check_still_active(DK3DInputHandler&) const override; // TODO
         virtual bool is_valid_parent() const override { return true; }

      protected:
         virtual node* _clone_impl() const override { return new editor_mode(this->mode); };
   };
}
