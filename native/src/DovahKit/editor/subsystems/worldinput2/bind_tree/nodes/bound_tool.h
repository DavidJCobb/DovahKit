#pragma once
#include "./abstract_input_node.h"
#include "../../enums/button_press_type.h"
#include "editor/subsystems/worldedit/tool_system/tool_id.h"

namespace dovahkit::subsystems::worldedit::tools {
   class opaque_options_union;
}
namespace dovahkit::subsystems::worldinput2 {
   class combined_tool_results;
}

namespace dovahkit::subsystems::worldinput2::binds::nodes {
   class bound_tool final : public abstract_input_node {
      public:
         static constexpr node_type my_type = node_type::bound_tool;
      public:
         bound_tool() : abstract_input_node(my_type) {}
         ~bound_tool();

         typename button_press_type button_press_type = button_press_type::press;
         struct {
            worldedit::tools::tool_id               id      = worldedit::tools::id_of_none;
            worldedit::tools::opaque_options_union* options = nullptr;
         } tool;

         virtual bool is_valid_parent() const override final { return false; }

      protected:
         virtual node* _clone_impl() const override final;
         virtual node* _read_impl(cobb::streams::bitreader&) override;
         virtual node* _write_impl(cobb::streams::bitwriter&) const override;
   };
}