#include "Options3DControlSchemeModel.h"
#include "dk3d/bind_tree/node.h"
#include "dk3d/bind_tree/nodes/editor_mode.h"
#include "dk3d/bind_tree/nodes/input.h"
#include "dk3d/bind_tree/nodes/root.h"
#include "dk3d/tools/_all.h"
#include "localization.h"

Options3DControlSchemeModel::Options3DControlSchemeModel(QObject* parent) : QAbstractItemModel(parent) {
   {
      static bool _registered = false;
      if (!_registered) {
         qRegisterMetaType<DK3D::inputs::bound_input>();
      }
   }
}
Options3DControlSchemeModel::~Options3DControlSchemeModel() {
}

void Options3DControlSchemeModel::setTree(const DK3D::binds::tree& tree) {
   this->beginResetModel();
   this->_state.tree = tree;
   this->endResetModel();
}

bool Options3DControlSchemeModel::is_real_root(const QModelIndex& qmi) const {
   //
   // In Qt's model system, the "root node" of a model isn't supposed to be an actual node. 
   // The built-in widgets are incapable of displaying it, so if you have data where the 
   // "root node" is an actual node with content and meaning, then you need to create an 
   // "imginary root node" to hold the "real root node." Naturally, Qt's documentation does 
   // not appear to mention this anywhere.
   //
   return qmi.internalPointer() == this && qmi.row() == 0;
}
QModelIndex Options3DControlSchemeModel::qmi_from_node(const DK3D::binds::node* node, int col) const {
   if (!node)
      return QModelIndex();
   if (node == this->_state.tree.root)
      return this->qmi_from_real_root(col);
   auto* parent = node->parent_node();
   if (!parent)
      return QModelIndex();
   return this->createIndex(parent->index_of(*node), col, parent);
}
DK3D::binds::node* Options3DControlSchemeModel::node_from_qmi(const QModelIndex& qmi) const {
   if (is_real_root(qmi))
      return this->_state.tree.root;
   if (!qmi.isValid())
      return nullptr;
   const auto* node = (const DK3D::binds::node*)qmi.internalPointer();
   if (!node)
      return nullptr;
   const auto& kids = node->child_nodes();
   auto i = qmi.row();
   if (i >= kids.size())
      return nullptr;
   return kids[i];
}

#pragma region QAbstractItemModel overrides
int Options3DControlSchemeModel::columnCount(const QModelIndex& parent) const {
   return 3;
}
QVariant Options3DControlSchemeModel::data(const QModelIndex& index, int role) const {
   auto* node = node_from_qmi(index);
   if (!node)
      return QVariant();
   auto col = index.column();
   switch (role) {
      case Qt::DisplayRole:
         if (col == 0) {
            if (node->type == DK3D::binds::node_type::root)
               return tr("Global", "root node name");
            if (auto* casted = node->as<DK3D::binds::nodes::editor_mode>()) {
               switch (casted->mode) {
                  using _ = DK3D::editor_mode;
                  case _::object:
                     return tr("Object Mode", "editor mode");
                  case _::landscape:
                     return tr("Landscape Mode", "editor mode");
                  case _::navmesh:
                     return tr("Navmesh Mode", "editor mode");
               }
               return tr("? Mode", "editor mode (unknown)");
            }
         }
         if (auto* casted = node->as<DK3D::binds::nodes::input>()) {
            switch (col) {
               case 0:
                  return casted->name;
               case 1: // Tool name
                  return DK3DLocalization::tool_name(casted->tool);
               case 2: // Mapped to
                  return DK3DLocalization::stringify_input(casted->mapping);
            }
         }
         break;
      case BoundInputRole:
         if (auto* casted = node->as<DK3D::binds::nodes::input>()) {
            return QVariant::fromValue(casted->mapping);
         }
         break;
      case InputNodeNameRole:
         if (auto* casted = node->as<DK3D::binds::nodes::input>()) {
            return casted->name;
         }
         break;
      case InputNodeToolRole:
         if (auto* casted = node->as<DK3D::binds::nodes::input>()) {
            if (!casted->tool)
               return (int)DK3D::tools::id_of_none;
            return (int) DK3D::all_tool_instances::get().id_of(*casted->tool);
         }
         break;
      case NodeTypeRole:
         return (int)node->type;
   }
   return QVariant();
}
Qt::ItemFlags Options3DControlSchemeModel::flags(const QModelIndex& index) const {
   auto* node = node_from_qmi(index);
   if (!node) {
      if (!index.isValid()) // imaginary root
         return Qt::ItemFlag::ItemIsEnabled;
      return 0;
   }
   switch (node->type) {
      using _ = DK3D::binds::node_type;
      case _::root:
         return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
      case _::editor_mode:
         return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
      case _::input:
         return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
   }
   return 0;
}
bool Options3DControlSchemeModel::hasChildren(const QModelIndex& parent) const {
   if (!parent.isValid()) // imaginary root
      return true;
   auto* node = node_from_qmi(parent);
   if (!node)
      return false;
   return !node->child_nodes().isEmpty();
}
QVariant Options3DControlSchemeModel::headerData(int section, Qt::Orientation orientation, int role) const {
   if (orientation == Qt::Orientation::Horizontal) {
      switch (section) {
         case 0:
            return tr("Name", "column header");
         case 1:
            return tr("Tool", "column header");
         case 2:
            return tr("Mapped to", "column header");
      }
   }
   return QVariant();
}
QModelIndex Options3DControlSchemeModel::index(int row, int column, const QModelIndex& parent_qmi) const {
   if (row < 0 || column < 0)
      return QModelIndex();
   if (!parent_qmi.isValid()) { // imaginary root; see comments in Options3DControlSchemeModel::is_real_root
      if (row == 0)
         return this->qmi_from_real_root(column);
      return QModelIndex();
   }
   auto* parent = node_from_qmi(parent_qmi);
   if (!parent)
      return QModelIndex();
   if (row >= parent->child_nodes().size())
      return QModelIndex();
   return this->createIndex(row, column, parent);
}
QModelIndex Options3DControlSchemeModel::parent(const QModelIndex& index) const {
   if (!index.isValid()) // imaginary root has no parent
      return QModelIndex();
   if (is_real_root(index)) // real root's parent is imaginary root
      return QModelIndex();
   auto* node = this->node_from_qmi(index);
   if (!node)
      return QModelIndex();
   if (node == this->_state.tree.root)
      return this->qmi_from_real_root(0);
   auto* parent = node->parent_node();
   return this->qmi_from_node(parent, 0);
}
int Options3DControlSchemeModel::rowCount(const QModelIndex& parent) const {
   if (!parent.isValid()) // imaginary root; see comments in Options3DControlSchemeModel::is_real_root
      return 1; // imaginary root contains real root
   auto* node = node_from_qmi(parent);
   if (!node)
      return 0;
   return node->child_nodes().size();
}
bool Options3DControlSchemeModel::setData(const QModelIndex& index, const QVariant& value, int role) {
   auto* node = node_from_qmi(index);
   if (!node)
      return false;
   switch (role) {
      case BoundInputRole:
         if (auto* casted = node->as<DK3D::binds::nodes::input>()) {
            if (value.canConvert<DK3D::inputs::bound_input>()) {
               auto v = value.value<DK3D::inputs::bound_input>();
               casted->mapping = v;
               return true;
            }
         }
         break;
      case InputNodeNameRole:
         if (auto* casted = node->as<DK3D::binds::nodes::input>()) {
            casted->name = value.toString();
            return true;
         }
         break;
      case InputNodeToolRole:
         if (auto* casted = node->as<DK3D::binds::nodes::input>()) {
            if (!value.canConvert<int>())
               break;
            DK3D::tool_id id   = value.toInt();
            const auto*   tool = DK3D::all_tool_instances::get()[id];
            if (casted->tool == tool)
               return true;
            casted->tool   = tool;
            casted->params = DK3D::tools::option_union::construct_for_type(id);;
            return true;
         }
         break;
   }
   return false;
}

bool Options3DControlSchemeModel::moveRows(const QModelIndex& sourceParent, int source_row, int count, const QModelIndex& destinationParent, int to_position) {
   if (source_row < 0 || count <= 0)
      return false;
   auto* parent_src = node_from_qmi(sourceParent);
   auto* parent_dst = node_from_qmi(destinationParent);
   if (!parent_src || !parent_dst || !parent_dst->is_valid_parent())
      return false;
   if (source_row + count >= parent_src->child_nodes().size())
      return false;
   int first = source_row;
   int last  = source_row + count - 1;
   //
   const auto& src_nodes = parent_src->child_nodes();
   if (last >= src_nodes.size()) // don't move anything past the end of the source
      return false;
   if (!this->beginMoveRows(sourceParent, first, last, destinationParent, to_position))
      return false;
   if (parent_dst == parent_src && to_position > source_row) {
      //
      // When moving items downward within the same parent, they are placed before the 
      // position denoted by (to_position).
      //
      --to_position;
      while (count--) {
         parent_dst->insert(*src_nodes[first], to_position);
      }
   } else {
      //
      // When moving items upward within the same parent or moving them across parents, 
      // the first moved item is placed in the position denoted by (to_position).
      //
      while (count--) {
         parent_dst->insert(*src_nodes[last], to_position);
      }
   }
   this->endMoveRows();
   return true;
}
bool Options3DControlSchemeModel::insertRows(int row, int count, const QModelIndex& qmi_parent) {
   if (row < 0 || count <= 0)
      return false;
   auto* parent = this->node_from_qmi(qmi_parent);
   if (!parent || !parent->is_valid_parent())
      return false;
   if (row > parent->child_nodes().size())
      return false;
   this->beginInsertRows(qmi_parent, row, row + count);
   while (count--) {
      auto* child = new DK3D::binds::nodes::input;
      parent->insert(*child, row);
   }
   this->endInsertRows();
   return true;
}
bool Options3DControlSchemeModel::removeRows(int row, int count, const QModelIndex& qmi_parent) {
   if (row < 0 || count <= 0)
      return false;
   auto* parent = this->node_from_qmi(qmi_parent);
   if (!parent)
      return false;
   if (row + count >= parent->child_nodes().size())
      return false;
   auto nodes = parent->child_nodes();
   this->beginRemoveRows(qmi_parent, row, row + count);
   for (int i = 0; i < count; ++i) {
      parent->remove(*nodes[row + i]);
   }
   this->endRemoveRows();
   return true;
}
#pragma endregion

void Options3DControlSchemeModel::nonCursedMoveRow(const QModelIndex& source_parent, int row, const QModelIndex& dest_parent, int down) {
   int to = row + down;
   if (to < 0)
      to = 0;
   if (down > 0)
      ++to;
   this->moveRow(source_parent, row, dest_parent, to);
}

//

#pragma region Options3DControlSchemeTreeModel
bool Options3DControlSchemeTreeModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const {
   auto* source = (Options3DControlSchemeModel*)this->sourceModel();
   if (!source)
      return false;
   if (!source_parent.isValid()) { // imaginary root or some other cursed garbage
      return true;
   }
   auto qmi = source->index(source_row, 0, source_parent);
   {
      auto dt = source->data(qmi, Options3DControlSchemeModel::NodeTypeRole);
      if (dt.isValid() && dt.type() == QMetaType::Int) {
         using _ = DK3D::binds::node_type;
         switch ((_)dt.value<int>()) {
            case _::editor_mode:
            case _::root:
               //
               // Nodes of these types should be displayed even if they are leaf nodes.
               //
               return true;
         }
      }
   }
   //
   // Hide leaf nodes.
   //
   return source->hasChildren(qmi);
}
#pragma endregion