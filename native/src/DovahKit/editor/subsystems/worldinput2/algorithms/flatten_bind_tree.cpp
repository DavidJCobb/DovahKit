#include "./flatten_bind_tree.h"
#include <cassert>
#include "../bind_tree/tree.h"
#include "../bind_tree/nodes/bound_tool.h"
#include "../bind_tree/nodes/editor_mode.h"
#include "../bind_tree/nodes/modifier.h"
#include "../bind_tree/nodes/root.h"

#include "editor/subsystems/worldedit/tool_system/options_union.h"

namespace dovahkit::subsystems::worldinput2::algorithms {
   extern bind_list flatten_bind_tree(
      const binds::tree& src
   ) {
      auto out = bind_list(src.device_type);

      std::optional<worldedit::editor_mode> current_editor_mode;

      auto traversal = [&out, &current_editor_mode](const binds::node& current) {
         auto recurse = [&](const binds::node& current, const auto& recurse) -> void {
            for (const auto* child : current.child_nodes()) {
               if (auto* bt = child->as<binds::nodes::bound_tool>()) {
                  auto& item = out.items.emplace_back();
                  item.name               = bt->name;
                  item.editor_mode        = current_editor_mode;
                  item.input_sequence     = bt->absolute_input_sequence();
                  item.button_press_type  = bt->button_press_type;
                  item.bound_tool.tool    = bt->tool.id;
                  item.bound_tool.options = ((worldedit::tools::options_union*)bt->tool.options)->clone();
                  continue;
               }

               #if _DEBUG
                  if (auto* casted = child->as<binds::nodes::modifier>()) {
                     assert(casted->button_press_type == button_press_type::hold);
                  }
               #endif

               if (!child->child_nodes().size())
                  continue;

               auto prior_editor_mode = current_editor_mode;
               if (auto* casted = child->as<binds::nodes::editor_mode>()) {
                  if (current_editor_mode.has_value()) {
                     if (current_editor_mode.value() != casted->mode) {
                        continue;
                     }
                  } else {
                     current_editor_mode = casted->mode;
                  }
               }
               recurse(*child, recurse);
               current_editor_mode = prior_editor_mode;
            }
         };
         recurse(current, recurse);
      };
      traversal(*src.root);

      return out;
   }
}