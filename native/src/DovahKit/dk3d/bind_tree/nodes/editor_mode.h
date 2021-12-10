#pragma once
#include "../node.h"
#include "../../enums/EditorMode.h"

namespace DK3D::binds::nodes {
   class editor_mode final : public node {
      public:
         static constexpr node_type my_type = node_type::editor_mode;
      public:
         editor_mode(EditorMode m) : node(my_type), mode(m) {}

         EditorMode mode;

         virtual bool check_still_active(DK3DInputHandler&) const override; // TODO
         virtual bool is_valid_parent() const override { return true; }

      protected:
         virtual node* _clone_impl() const override { return new editor_mode(this->mode); };
   };
}
