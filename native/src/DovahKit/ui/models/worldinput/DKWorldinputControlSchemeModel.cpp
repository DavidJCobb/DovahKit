#include "./DKWorldinputControlSchemeModel.h"

#include "editor/subsystems/worldinput2/bind_tree/nodes/abstract_input_node.h"
#include "editor/subsystems/worldinput2/bind_tree/nodes/bound_tool.h"
#include "editor/subsystems/worldinput2/bind_tree/nodes/editor_mode.h"
#include "editor/subsystems/worldinput2/bind_tree/nodes/modifier.h"
#include "editor/subsystems/worldinput2/bind_tree/nodes/root.h"

#include "editor/subsystems/worldinput2/algorithms/input_sequence_stringification.h"

#include "ui/options_window/worldedit_tools/get_worldedit_tool_info.h"

namespace {
   namespace worldedit {
      using namespace dovahkit::subsystems::worldedit;
   }
   namespace worldinput2 {
      using namespace dovahkit::subsystems::worldinput2;
   }

   void _recache_data(DKGenericTreeModelNode<DKWorldinputControlSchemeModel>& model_node) {
      auto* data_node = model_node.underlying_data;
      if (!data_node) {
         model_node.cached.name.clear();
         model_node.cached.bound_tool_name.clear();
         model_node.cached.input_sequence.clear();
         return;
      }

      model_node.cached.input_sequence.clear();
      if (auto* casted = data_node->as<worldinput2::binds::nodes::abstract_input_node>()) {
         model_node.cached.name = casted->name;

         auto& dst = model_node.cached.input_sequence;

         std::string is;
         worldinput2::algorithms::input_sequence_to_string(casted->input_sequence, is);

         dst = QString::fromStdString(is);

         if (auto* bt = casted->as<worldinput2::binds::nodes::bound_tool>()) {
            QString prefix;
            switch (bt->button_press_type) {
               case worldinput2::button_press_type::press:
                  prefix = DKWorldinputControlSchemeModel::tr("Press ", "button press type prefix");
                  break;
               case worldinput2::button_press_type::long_press:
                  prefix = DKWorldinputControlSchemeModel::tr("Long Press ", "button press type prefix");
                  break;
               case worldinput2::button_press_type::hold:
                  prefix = DKWorldinputControlSchemeModel::tr("Hold ", "button press type prefix");
                  break;
            }
            dst = prefix + dst;
         }
      }

      model_node.cached.bound_tool_name.clear();
      if (auto* casted = data_node->as<worldinput2::binds::nodes::bound_tool>()) {
         if (casted->tool.id != worldedit::tools::id_of_none) {
            if (const auto* info = worldedit_tool_info::info_of(casted->tool.id)) {
               model_node.cached.bound_tool_name = info->name;
            }
         }
      }
   }
}

void DKWorldinputControlSchemeModel::on_before_delete_node(node_type& node) {
}
void DKWorldinputControlSchemeModel::on_before_delete_root() {
}

QVariant DKWorldinputControlSchemeModel::data_of(const node_type& node, Qt::ItemDataRole role, size_t column) const {
   using node_type_enum = node_type::node_type_enum;

   if (!node.underlying_data)
      return {};
   auto type = node.underlying_data->type;

   if (role == Qt::ItemDataRole::DisplayRole || role == Qt::ItemDataRole::ToolTipRole) {
      switch (type) {
         case node_type_enum::bound_tool:
         case node_type_enum::modifier:
            switch (column) {
               case Columns::Name:
                  return node.cached.name;
               case Columns::Behavior:
                  if (type == node_type_enum::modifier) {
                     return tr("Modifier Key", "node typename");
                  }
                  return node.cached.bound_tool_name;
               case Columns::Value:
                  return node.cached.input_sequence;
            }
            break;
         case node_type_enum::editor_mode:
            switch (column) {
               case Columns::Name:
                  return tr("Mode-specific binds", "node typename");
               case Columns::Value:
                  {
                     const auto* em = (dovahkit::subsystems::worldinput2::binds::nodes::editor_mode*)node.underlying_data;
                     switch (em->mode) {
                        using enum dovahkit::subsystems::worldedit::editor_mode;
                        case objects:
                           return tr("Object Mode", "editor mode");
                        case terrain:
                           return tr("Landscape Mode", "editor mode");
                        case navmesh:
                           return tr("Navmesh Mode", "editor mode");
                     }
                  }
                  return tr("???", "editor mode");
            }
            break;
         case node_type_enum::root:
            if (column == Columns::Name)
               return tr("Control scheme root", "node typename");
      }
      return {};
   }
   return {};
}
Qt::ItemFlags DKWorldinputControlSchemeModel::flags_of(const node_type& node, size_t column) const {
   using node_type_enum = node_type::node_type_enum;

   if (!node.underlying_data)
      return {};
   auto type = node.underlying_data->type;

   Qt::ItemFlags flags = {};
   flags |= Qt::ItemFlag::ItemIsEnabled;
   flags |= Qt::ItemFlag::ItemIsSelectable;
   if (type == node_type_enum::bound_tool) {
      flags |= Qt::ItemFlag::ItemNeverHasChildren;
   }
   return flags;
}

const DKWorldinputControlSchemeModel::node_type* DKWorldinputControlSchemeModel::root_node() const noexcept {
   if (this->invisible_root.child_count())
      return this->invisible_root.nth_child(0);
   return nullptr;
}

#pragma region QAbstractItemModel overrides
   #pragma region Node data
      QVariant DKWorldinputControlSchemeModel::headerData(int section, Qt::Orientation orientation, int role) const {
         if (role == Qt::DisplayRole) {
            switch (section) {
               case Columns::Name:
                  return tr("Name", "column heading");
               case Columns::Behavior:
                  return tr("Behavior", "column heading");
               case Columns::Value:
                  return tr("Details", "column heading");
            }
         }
         return {};
      }
   #pragma endregion
#pragma endregion

void DKWorldinputControlSchemeModel::overwriteFromSource(const control_scheme_type& src) {
   node_type* new_root = nullptr;

   auto clone = [this, &src](const underlying_node_type& g) -> node_type* {
      auto recurse = [&](const underlying_node_type& g, node_type* parent, auto& recurse) -> node_type* {
         auto* node = new node_type;
         node->underlying_data = g.clone();
         _recache_data(*node);

         if (g.type != node_type::node_type_enum::bound_tool) {
            for (const auto* child : g.child_nodes()) {
               assert(child != nullptr);
               node->append_child(*recurse(*child, node, recurse));
            }
         }
         return node;
      };
      return recurse(g, nullptr, recurse);
   };
   if (src.root)
      new_root = clone(*src.root);

   this->replaceAll(new_root);
}
void DKWorldinputControlSchemeModel::overwriteDestination(control_scheme_type& dst) const {
   if (dst.root) {
      delete dst.root;
      dst.root = nullptr;
   }

   auto clone = [this, &dst](const node_type& n) {
      auto recurse = [&](const node_type& n, underlying_node_type* parent, auto& recurse) -> void {
         if (!n.underlying_data)
            return;

         auto* clone = n.underlying_data->shallow_clone();
         if (parent)
            parent->append(*clone);
         else {
            assert(clone->type == node_type::node_type_enum::root);
            dst.root = (worldinput2::binds::nodes::root*)clone;
         }

         if (n.underlying_data->type != node_type::node_type_enum::bound_tool) {
            for (const node_type* child : n.children()) {
               assert(child != nullptr);
               recurse(*child, clone, recurse);
            }
         }
      };
      recurse(n, nullptr, recurse);
   };
   if (this->invisible_root.child_count())
      clone(*this->invisible_root.nth_child(0));
}