#include "./DKWorldinputInputSequenceModel.h"
#include <QIcon>

void DKWorldinputInputSequenceModel::on_before_delete_node(node_type& node) {
   if (&node == this->_raycast_associated_button)
      this->_raycast_associated_button = nullptr;
}
void DKWorldinputInputSequenceModel::on_before_delete_root() {
   this->_raycast_associated_button = nullptr;
}

QVariant DKWorldinputInputSequenceModel::data_of(const node_type& node, Qt::ItemDataRole role, size_t column) const {
   if (column == Columns::Name) {
      switch (role) {
         case Qt::ItemDataRole::DisplayRole:
         case Qt::ItemDataRole::ToolTipRole:
            switch (node.type) {
               case group_type::concurrent_ordered:
                  return tr("Concurrent and ordered group", "group type");
               case group_type::concurrent_unordered:
                  return tr("Concurrent and unordered group", "group type");
               case group_type::separated_ordered:
                  return tr("Separate and ordered group", "group type");
            }
            return node.button_name();
      }
   }
   if (column == Columns::RaycastAssociatedIndicator) {
      switch (role) {
         case Qt::ItemDataRole::DecorationRole:
            if (&node == this->_raycast_associated_button) {
               return QIcon(":/icons/star.png");
            }
            break;
         case Qt::ItemDataRole::SizeHintRole:
            return QSize(16, 16);
      }
   }
   switch (role) {
      case IsRaycastAssociatedRole:
         return &node == this->_raycast_associated_button;
      case GroupTypeRole:
         return (int)node.type;
   }
   return {};
}
Qt::ItemFlags DKWorldinputInputSequenceModel::flags_of(const node_type& node, size_t column) const {
   Qt::ItemFlags flags = {};
   flags |= Qt::ItemFlag::ItemIsEnabled;
   flags |= Qt::ItemFlag::ItemIsSelectable;
   if (node.type == group_type::single_control) {
      flags |= Qt::ItemFlag::ItemNeverHasChildren;
   }
   return flags;
}

const DKWorldinputInputSequenceModel::node_type* DKWorldinputInputSequenceModel::root_node() const noexcept {
   if (this->invisible_root.child_count())
      return this->invisible_root.nth_child(0);
   return nullptr;
}

#pragma region QAbstractItemModel overrides
   #pragma region Node data
      QVariant DKWorldinputInputSequenceModel::headerData(int section, Qt::Orientation orientation, int role) const {
         if (section == Columns::Name && role == Qt::DisplayRole)
            return tr("Input sequence");
         if (section == Columns::RaycastAssociatedIndicator) {
            switch (role) {
               case Qt::ItemDataRole::DisplayRole:
                  return tr("R.A.", "column header for raycast-associated-button indicator");
               case Qt::ItemDataRole::ToolTipRole:
                  return tr("The raycast-associated button will have a star icon in this column.");
               case Qt::ItemDataRole::SizeHintRole:
                  return QSize(16, 16);
            }
         }
         return {};
      }
   #pragma endregion
#pragma endregion

void DKWorldinputInputSequenceModel::overwriteFromSource(const input_sequence& src) {
   node_type* new_root = nullptr;

   auto clone = [this, &src](const input_sequence::group& g) -> node_type* {
      auto recurse = [&](const input_sequence::group& g, node_type* parent, auto& recurse) -> node_type* {
         auto* node = new node_type;
         node->type = g.type;

         if (g.type == group_type::single_control) {
            node->button = g.button;
         }
         if (&g == src.raycast.associated_button) {
            this->_raycast_associated_button = node;
         }
         if (g.can_have_children()) {
            for (const auto* child : g.children) {
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
void DKWorldinputInputSequenceModel::overwriteDestination(input_sequence& dst) const {
   dst.raycast.associated_button = nullptr;
   if (dst.root) {
      delete dst.root;
      dst.root = nullptr;
   }

   auto clone = [this, &dst](const node_type& n) {
      auto recurse = [&](const node_type& n, input_sequence::group* parent, auto& recurse) -> void {
         auto* group = new input_sequence::group;
         group->type = n.type;
         if (parent) {
            parent->children.push_back(group);
         } else {
            assert(!dst.root);
            dst.root = group;
         }

         if (n.type == group_type::single_control) {
            group->button = n.button;
            if (&n == this->_raycast_associated_button) {
               dst.raycast.associated_button = group;
            }

            assert(n.child_count() == 0);
         } else {
            assert(&n != this->_raycast_associated_button);

            for (const node_type* child : n.children()) {
               assert(child != nullptr);
               recurse(*child, group, recurse);
            }
         }
      };
      recurse(n, nullptr, recurse);
   };
   if (this->invisible_root.child_count())
      clone(*this->invisible_root.nth_child(0));
}

bool DKWorldinputInputSequenceModel::_insertInOrAfter(const QModelIndex& target, node_type* item) {
   auto* target_node = this->node(target);

   if (!target_node) {
      assert(this->root_node() == nullptr);
      this->beginInsertRows({}, 0, 0);
      this->invisible_root.append_child(*item);
      this->endInsertRows();
      return true;
   }

   auto* parent_node = target_node;
   int   insert_at   = parent_node->child_count();
   if (!parent_node->can_have_children()) {

      if (parent_node == this->root_node()) {
         //
         // Special case: the root node is a leaf node. There's no way to insert other 
         // nodes inside of a leaf, nor a parent node to insert a sibling into. Instead, 
         // create a new node to wrap both the root node and the node we wish to insert, 
         // and make this wrapper the new root node.
         //
         this->beginResetModel();
         auto* wrap = new node_type;
         wrap->type = group_type::concurrent_ordered;
         wrap->append_child(*parent_node);
         wrap->append_child(*item);
         this->invisible_root.append_child(*wrap);
         assert(this->invisible_root.child_count() == 1);
         this->endResetModel();
         return true;
      }

      parent_node = parent_node->parent_node();
      if (!parent_node)
         return false;
      insert_at = parent_node->index_of_child(*target_node) + 1;
   } else {
      insert_at = parent_node->child_count();
   }
   
   auto first_inserted = insert_at;
   auto last_inserted  = insert_at;
   this->beginInsertRows(this->index(parent_node), first_inserted, last_inserted);
   parent_node->insert_child(insert_at, *item);
   this->endInsertRows();
   return true;
}

std::optional<QModelIndex> DKWorldinputInputSequenceModel::addButtonTo(const QModelIndex& parent) {
   //
   // This returns std::optional because we use invalid indices for both the 
   // simulated QTreeView root and our actual real root node, so the isValid 
   // member function on QModelIndex is worthless to the caller.
   //
   auto* item = new node_type();
   item->type = group_type::single_control;
   //
   if (!this->_insertInOrAfter(parent, item)) {
      delete item;
      return {};
   }
   return this->index(item);
}
std::optional<QModelIndex> DKWorldinputInputSequenceModel::addGroupTo(const QModelIndex& parent) {
   auto* item = new node_type();
   item->type = group_type::concurrent_ordered;
   //
   if (!this->_insertInOrAfter(parent, item)) {
      delete item;
      return {};
   }
   return this->index(item);
}

std::optional<DKWorldinputInputSequenceModel::NodeInfo> DKWorldinputInputSequenceModel::infoFor(const QModelIndex& qmi) const {
   auto* node = this->node(qmi);
   if (!node)
      return {};
   return NodeInfo{
      .type   = node->type,
      .button = node->button,
   };
}
void DKWorldinputInputSequenceModel::replaceInfoFor(const QModelIndex& qmi, const NodeInfo& info) {
   auto* node = this->node(qmi);
   if (!node)
      return;

   if (node->type == group_type::single_control) {
      assert(info.type == node->type);
      node->button = info.button;
   } else {
      assert(info.type != group_type::single_control);
      node->type = info.type;
   }
   this->emitNodeChanged(*node);
}

std::optional<QModelIndex> DKWorldinputInputSequenceModel::raycastAssociatedButton() const {
   if (!this->_raycast_associated_button)
      return {};
   return this->index(this->_raycast_associated_button);
}
bool DKWorldinputInputSequenceModel::isRaycastAssociatedButton(const QModelIndex& qmi) const {
   if (!this->_raycast_associated_button)
      return false;
   return this->node(qmi) == this->_raycast_associated_button;
}
bool DKWorldinputInputSequenceModel::setRaycastAssociatedButton(const QModelIndex& qmi) {
   auto* prior = this->_raycast_associated_button;
   auto* node  = this->node(qmi);
   if (!node) {
      this->_raycast_associated_button = nullptr;
      if (prior) {
         this->emitNodeChanged(*prior, Columns::RaycastAssociatedIndicator);
      }
      return true;
   }
   if (node->type != group_type::single_control)
      return false;
   this->_raycast_associated_button = node;
   if (prior) {
      this->emitNodeChanged(*prior, Columns::RaycastAssociatedIndicator);
   }
   if (node) {
      auto qmi_ra = qmi.siblingAtColumn(Columns::RaycastAssociatedIndicator);
      emit dataChanged(qmi_ra, qmi_ra);
   }
   return true;
}