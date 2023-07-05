#include "./DKWorldinputControlSchemeModel.h"

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
      auto& data = model_node.data;
      if (std::holds_alternative<std::monostate>(data)) {
         model_node.cached.name.clear();
         model_node.cached.bound_tool_name.clear();
         model_node.cached.input_sequence.clear();
         return;
      }

      model_node.cached.bound_tool_name.clear();
      model_node.cached.input_sequence.clear();
      //
      auto cache_sequence = [&model_node](const worldinput2::input_sequence& seq) {
         std::string is;
         worldinput2::algorithms::input_sequence_to_string(seq, is);

         model_node.cached.input_sequence = QString::fromStdString(is);
      };
      //
      if (auto* casted = std::get_if<worldinput2::control_scheme_action>(&data)) {
         model_node.cached.name = casted->name;

         cache_sequence(casted->input_sequence);
         {
            QString prefix;
            switch (casted->button_press_type) {
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
            model_node.cached.input_sequence.prepend(prefix);
         }

         if (casted->tool.id != worldedit::tools::id_of_none) {
            if (const auto* info = worldedit_tool_info::info_of(casted->tool.id)) {
               model_node.cached.bound_tool_name = info->name;
            }
         }
      } else if (auto* casted = std::get_if<worldinput2::control_scheme_modifier>(&data)) {
         cache_sequence(casted->input_sequence);
      }
   }
}

void DKWorldinputControlSchemeModel::on_before_delete_node(node_type& node) {
}
void DKWorldinputControlSchemeModel::on_before_delete_root() {
}

QVariant DKWorldinputControlSchemeModel::data_of(const node_type& node, Qt::ItemDataRole role, size_t column) const {
   auto& data = node.data;
   if (std::holds_alternative<std::monostate>(data))
      return {};

   if (role == Qt::ItemDataRole::DisplayRole || role == Qt::ItemDataRole::ToolTipRole) {
      if (std::holds_alternative<worldinput2::control_scheme_action>(data) || std::holds_alternative<worldinput2::control_scheme_modifier>(data)) {
         switch (column) {
            case Columns::Name:
               return node.cached.name;
            case Columns::Behavior:
               if (std::holds_alternative<worldinput2::control_scheme_modifier>(data)) {
                  return tr("Modifier Key", "node typename");
               }
               return node.cached.bound_tool_name;
            case Columns::Value:
               return node.cached.input_sequence;
         }
         return {};
      } else if (std::holds_alternative<worldinput2::control_scheme_condition>(data)) {
         auto& casted = std::get<worldinput2::control_scheme_condition>(data);

         switch (column) {
            case Columns::Name:
               return tr("Mode-specific binds", "node typename");
            case Columns::Value:
               switch (casted.mode) {
                  using enum dovahkit::subsystems::worldedit::editor_mode;
                  case objects:
                     return tr("Object Mode", "editor mode");
                  case terrain:
                     return tr("Landscape Mode", "editor mode");
                  case navmesh:
                     return tr("Navmesh Mode", "editor mode");
               }
               return tr("???", "editor mode");
         }
         return {};
      }
   }
   return {};
}
Qt::ItemFlags DKWorldinputControlSchemeModel::flags_of(const node_type& node, size_t column) const {
   if (std::holds_alternative<std::monostate>(node.data))
      return {};

   Qt::ItemFlags flags = {};
   flags |= Qt::ItemFlag::ItemIsEnabled;
   flags |= Qt::ItemFlag::ItemIsSelectable;
   if (!node.can_have_children()) {
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
   auto clone = [](const control_scheme_type::node& g) -> node_type* {
      auto recurse = [](const control_scheme_type::node& g, auto& recurse) -> node_type* {
         auto* node = new node_type;
         node->data = cobb::node_to_variant<node_type::variant_type>(g);
         _recache_data(*node);

         if (g.can_have_children()) {
            for (const auto* child : g.children) {
               assert(child != nullptr);
               node->append_child(*recurse(*child, recurse));
            }
         }
         return node;
      };
      return recurse(g, recurse);
   };

   this->clear();
   for (const auto* tpn : src.top_level_nodes)
      this->invisible_root.append_child(*clone(*tpn));
}
void DKWorldinputControlSchemeModel::overwriteDestination(control_scheme_type& dst) const {
   dst.clear();

   auto clone = [this, &dst](const node_type& n) {
      auto recurse = [&](const node_type& n, control_scheme_type::node* parent, auto& recurse) -> void {
         if (std::holds_alternative<std::monostate>(n.data))
            return;

         auto* clone = cobb::node_from_variant<control_scheme_type::node>(n.data);
         if (parent)
            parent->append_child(*clone);
         else {
            dst.top_level_nodes.push_back(clone);
         }

         if (n.can_have_children()) {
            for (const node_type* child : n.children()) {
               assert(child != nullptr);
               recurse(*child, clone, recurse);
            }
         }
      };
      recurse(n, nullptr, recurse);
   };
   for (auto* tpn : this->invisible_root.children()) {
      clone(*tpn);
   }
}

DKWorldinputControlSchemeModel::node_type::variant_type DKWorldinputControlSchemeModel::infoFor(const QModelIndex& qmi) const {
   auto* n = this->node(qmi);
   if (!n)
      return {};
   return n->data;
}
void DKWorldinputControlSchemeModel::replaceInfoFor(const QModelIndex& qmi, const node_type::variant_type& data) {
   auto* n = this->node(qmi);
   if (!n)
      return;
   n->data = data;
   _recache_data(*n);
   this->emitNodeChanged(qmi);
}