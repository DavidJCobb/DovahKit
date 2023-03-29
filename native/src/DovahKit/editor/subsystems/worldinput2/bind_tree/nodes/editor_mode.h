#pragma once
#include "../node.h"
#include "editor/subsystems/worldedit/enums/editor_mode.h"

namespace dovahkit::subsystems::worldinput2::binds::nodes {
   class editor_mode final : public node {
      public:
         static constexpr node_type my_type = node_type::editor_mode;
         using value_type = ::dovahkit::subsystems::worldedit::editor_mode;

      public:
         editor_mode(value_type m) : node(my_type), mode(m) {}

         value_type mode;

         virtual bool is_valid_parent() const override { return true; }

      protected:
         virtual node* _clone_impl() const override { return new editor_mode(this->mode); };
   };
}
