#include "./DKWorldinputInputSequenceModel.h"
#include <QIcon>

/*
  
   RULES FOR QModelIndex HANDLING:
   
    - A QMI refers to the Nth child of a node X, given N as the row number and a 
      pointer to X as the internal pointer.
   
    - An index with negative row or column numbers refers to the tree's invisible root.
   
    - An index with a null internal pointer and row == 0 refers to the actual root.

   This requires some extra handling:

    - Methods like rowCount must interpret a QMI with no pointer as referring to the 
      invisible root, and so return a count of 1.
   
*/

DKWorldinputInputSequenceModel::DKWorldinputInputSequenceModel(QObject* parent) : QAbstractItemModel(parent) {
}

DKWorldinputInputSequenceModel::Node::~Node() {
   for (auto* item : this->group.children)
      if (item)
         delete item;
   this->group.children.clear();
}
void DKWorldinputInputSequenceModel::Node::append_child(Node& c) {
   assert(this->can_have_children());
   if (c.parent) {
      c.parent->remove_child(c);
   }
   c.parent = this;
   this->group.children.append(&c);
}
int DKWorldinputInputSequenceModel::Node::index_of_child(const Node& c) const {
   if (!this->can_have_children())
      return -1;
   return this->group.children.indexOf((Node*) &c);
}
void DKWorldinputInputSequenceModel::Node::insert_child(int i, Node& c) {
   assert(this->can_have_children());
   assert(i >= 0 && i <= this->child_count());
   if (c.parent) {
      c.parent->remove_child(c);
   }
   c.parent = this;
   this->group.children.insert(i, &c);
}
void DKWorldinputInputSequenceModel::Node::remove_child(Node& c) {
   assert(this->can_have_children());
   auto i = this->group.children.indexOf(&c);
   if (i < 0)
      return;
   this->group.children.remove(i);
   c.parent = nullptr;
}

QString DKWorldinputInputSequenceModel::Node::button_name() const {
   switch (this->button.mouse) {
      case Qt::MouseButton::LeftButton:
         return tr("LMB", "mouse button");
      case Qt::MouseButton::RightButton:
         return tr("RMB", "mouse button");
      case Qt::MouseButton::MiddleButton:
         return tr("MMB", "mouse button");
      case Qt::MouseButton::XButton1:
         return tr("Mouse X1", "mouse button");
      case Qt::MouseButton::XButton2:
         return tr("Mouse X2", "mouse button");
      case Qt::MouseButton::ExtraButton3:
         return tr("Mouse X3", "mouse button");
      case Qt::MouseButton::ExtraButton4:
         return tr("Mouse X4", "mouse button");
      case Qt::MouseButton::ExtraButton5:
         return tr("Mouse X5", "mouse button");
      case Qt::MouseButton::ExtraButton6:
         return tr("Mouse X6", "mouse button");
      case Qt::MouseButton::ExtraButton7:
         return tr("Mouse X7", "mouse button");
      case Qt::MouseButton::ExtraButton8:
         return tr("Mouse X8", "mouse button");
      case Qt::MouseButton::ExtraButton9:
         return tr("Mouse X9", "mouse button");
      case Qt::MouseButton::ExtraButton10:
         return tr("Mouse X10", "mouse button");
      case Qt::MouseButton::ExtraButton11:
         return tr("Mouse X11", "mouse button");
      case Qt::MouseButton::ExtraButton12:
         return tr("Mouse X12", "mouse button");
      case Qt::MouseButton::ExtraButton13:
         return tr("Mouse X13", "mouse button");
      case Qt::MouseButton::ExtraButton14:
         return tr("Mouse X14", "mouse button");
      case Qt::MouseButton::ExtraButton15:
         return tr("Mouse X15", "mouse button");
      case Qt::MouseButton::ExtraButton16:
         return tr("Mouse X16", "mouse button");
      case Qt::MouseButton::ExtraButton17:
         return tr("Mouse X17", "mouse button");
      case Qt::MouseButton::ExtraButton18:
         return tr("Mouse X18", "mouse button");
      case Qt::MouseButton::ExtraButton19:
         return tr("Mouse X19", "mouse button");
      case Qt::MouseButton::ExtraButton20:
         return tr("Mouse X20", "mouse button");
      case Qt::MouseButton::ExtraButton21:
         return tr("Mouse X21", "mouse button");
      case Qt::MouseButton::ExtraButton22:
         return tr("Mouse X22", "mouse button");
      case Qt::MouseButton::ExtraButton23:
         return tr("Mouse X23", "mouse button");
      case Qt::MouseButton::ExtraButton24:
         return tr("Mouse X24", "mouse button");
   }
   switch (this->button.xinput) {
      case xinput_button::a: return tr("A", "gamepad button");
      case xinput_button::b: return tr("B", "gamepad button");
      case xinput_button::x: return tr("X", "gamepad button");
      case xinput_button::y: return tr("Y", "gamepad button");
      case xinput_button::start: return tr("Start", "gamepad button");
      case xinput_button::back: return tr("Back", "gamepad button");
      case xinput_button::lb: return tr("Left Bumper", "gamepad button");
      case xinput_button::rb: return tr("Right Bumper", "gamepad button");
      case xinput_button::ls: return tr("Left Stick Click", "gamepad button");
      case xinput_button::rs: return tr("Right Stick Click", "gamepad button");
      case xinput_button::lt: return tr("Left Trigger", "gamepad button");
      case xinput_button::rt: return tr("Right Trigger", "gamepad button");
      case xinput_button::d_pad_up: return tr("D-Pad Up", "gamepad button");
      case xinput_button::d_pad_down: return tr("D-Pad Down", "gamepad button");
      case xinput_button::d_pad_left: return tr("D-Pad Left", "gamepad button");
      case xinput_button::d_pad_right: return tr("D-Pad Right", "gamepad button");
   }

   if (this->button.vk == virtual_key::none)
      return tr("<None>", "no key mapped");

   cobb::keyboard::key k(this->button.vk);
   k.make_complete();

   std::wstring name = k.get_key_name();
   return QString("Key: ") + QString::fromStdWString(name);
}

bool DKWorldinputInputSequenceModel::_is_empty_qmi(const QModelIndex& qmi) const {
   return (qmi.row() < 0 || qmi.column() < 0);
}
const DKWorldinputInputSequenceModel::Node* DKWorldinputInputSequenceModel::_node_from_qmi(const QModelIndex& qmi) const {
   if (qmi.row() < 0 || qmi.column() < 0)
      return nullptr;
   if (qmi.internalPointer() == nullptr) {
      if (qmi.row() == 0)
         return this->_root;
      return nullptr;
   }
   const Node* group = (const Node*)qmi.internalPointer();
   if (!group)
      return nullptr;
   assert(group->type != group_type::single_control);
   return group->group.children[qmi.row()];
}
QModelIndex DKWorldinputInputSequenceModel::_qmi_for_node(const Node* node, int col) const noexcept {
   if (!node)
      return {};
   if (node == this->_root)
      return this->createIndex(0, col, nullptr);
   auto* parent = node->parent;
   if (!parent)
      return {};
   return this->createIndex(parent->index_of_child(*node), col, parent);
}

void DKWorldinputInputSequenceModel::_clear_silent() {
   if (this->_root) {
      delete this->_root;
      this->_root = nullptr;
   }
   this->_raycast_associated_button = nullptr;
}

#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      QModelIndex DKWorldinputInputSequenceModel::index(int row, int column, const QModelIndex& parent) const {
         if (!this->_root || row < 0 || column < 0)
            return {};
         if (column > MaxColumns)
            return {};

         const auto* parent_node = this->_node_from_qmi(parent);
         if (!parent_node) {
            if (row == 0)
               return this->createIndex(row, column, nullptr);
            return {};
         }
         if (row >= parent_node->child_count())
            return {};

         return this->createIndex(row, column, (void*)parent_node);
      }
      QModelIndex DKWorldinputInputSequenceModel::parent(const QModelIndex& index) const {
         auto* node = this->_node_from_qmi(index);
         if (!node)
            return {};
         return this->_qmi_for_node(node->parent, index.column());
      }
      QModelIndex DKWorldinputInputSequenceModel::sibling(int row, int column, const QModelIndex& index) const {
         if (row < 0 || column < 0)
            return {};
         if (column > MaxColumns)
            return {};
         auto* node = this->_node_from_qmi(index);
         if (!node)
            return {};

         if (row != index.row()) {
            auto* parent = node->parent;
            if (!parent) {
               assert(node == this->_root);
               return {};
            }
            if (row >= parent->child_count())
               return {};
         }
         return this->createIndex(row, column, index.internalPointer());
      }
      int DKWorldinputInputSequenceModel::rowCount(const QModelIndex& parent) const {
         auto* parent_node = this->_node_from_qmi(parent);
         if (!parent_node)
            return 1;
         return parent_node->group.children.size();
      }
      int DKWorldinputInputSequenceModel::columnCount(const QModelIndex& item) const {
         return MaxColumns;
      }

      bool DKWorldinputInputSequenceModel::moveRows(const QModelIndex& from_parent, int first_row_index, int count, const QModelIndex& to_parent, int to_position) {
         //
         // This API's design is unintuitive. You probably assume that these arguments have the following 
         // meanings:
         //
         //  - `first_row_index`, or `sourceRow` in Qt's docs, is the index of the first row we want 
         //    to move.
         //
         //  - `count` is the number of elements we want to move, including the first one (so it must 
         //    be at least one).
         //
         //  - `to_position`, or `destinationChild` in Qt's docs, is the index to which we want to 
         //    move the first of the rows we're moving; the first row should have this index after 
         //    the move operation is complete.
         // 
         // Unfortunately, this isn't always the case. If you're moving items downward within the same 
         // parent, then the (to_position) argument is actually the index *above* which the *last* of 
         // the rows-to-be-moved should be placed.
         //
         if (first_row_index < 0 || count <= 0 || to_position < 0) // don't move anything to/from before the start of the table
            return false;

         auto* src_parent = this->_node_from_qmi(from_parent);
         auto* dst_parent = this->_node_from_qmi(to_parent);

         if (!src_parent || !dst_parent)
            return false;

         auto  src_size = src_parent->child_count();
         auto  dst_size = dst_parent->child_count();
         if (to_position > dst_size) // don't move anything past the end of the table
            return false;

         auto last_to_move = first_row_index + count - 1;
         if (last_to_move >= src_size) // don't move anything past the end of the table
            return false;

         //
         // Let Qt run its own correctness checks and then run its preparations. Among other things, 
         // this should check to make sure that the target position isn't in the middle of the range of 
         // rows you're moving.
         //
         if (!beginMoveRows(from_parent, first_row_index, last_to_move, to_parent, to_position))
            return false;

         auto& src_list = src_parent->group.children;
         auto& dst_list = dst_parent->group.children;
         if (src_parent != dst_parent) {
            for (int i = last_to_move; i >= first_row_index; --i) {
               auto* child = src_list[i];
               child->parent = dst_parent;
               dst_list.insert(to_position, child);
            }
            src_list.remove(first_row_index, count);
         } else {
            int target = last_to_move;
            if (to_position >= first_row_index) { // are we moving elements down?
               target = first_row_index;
               //
               // We also have to decrement (to_position) only when moving down, to make up for moveRows 
               // being completely cursed.
               //
               // NOT decrementing this means that you'll move items down one too far. ALWAYS decrementing 
               // this means that trying to move an item from index 1 to index 0 will crash, unless you 
               // just straight-up don't even allow that, which is QListWidget's approach.
               //
               --to_position;
            }
            while (count--)
               dst_list.move(target, to_position);
         }

         endMoveRows();
         return true;
      }
   #pragma endregion
   #pragma region Node data
      QVariant DKWorldinputInputSequenceModel::data(const QModelIndex& index, int role) const {
         const auto* node = this->_node_from_qmi(index);
         if (!node)
            return {};
         auto col = index.column();
         if (col == Columns::Name) {
            switch (role) {
               case Qt::ItemDataRole::DisplayRole:
               case Qt::ItemDataRole::ToolTipRole:
                  switch (node->type) {
                     case group_type::concurrent_ordered:
                        return tr("Concurrent and ordered group", "group type");
                     case group_type::concurrent_unordered:
                        return tr("Concurrent and unordered group", "group type");
                     case group_type::separated_ordered:
                        return tr("Separate and ordered group", "group type");
                  }
                  return node->button_name();
            }
         }
         if (col == Columns::RaycastAssociatedIndicator) {
            switch (role) {
               case Qt::ItemDataRole::DecorationRole:
                  if (node == this->_raycast_associated_button) {
                     return QIcon(":/icons/star.png");
                  }
                  break;
               case Qt::ItemDataRole::SizeHintRole:
                  return QSize(16, 16);
            }
         }
         switch (role) {
            case IsRaycastAssociatedRole:
               return node == this->_raycast_associated_button;
            case GroupTypeRole:
               return (int)node->type;
         }
         return {};
      }
      Qt::ItemFlags DKWorldinputInputSequenceModel::flags(const QModelIndex& index) const {
         Qt::ItemFlags flags = {};
         if (auto* node = this->_node_from_qmi(index)) {
            flags |= Qt::ItemFlag::ItemIsEnabled;
            flags |= Qt::ItemFlag::ItemIsSelectable;
            if (!node->can_have_children()) {
               //flags |= Qt::ItemFlag::ItemNeverHasChildren;
            }
         }
         return flags;
      }
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

void DKWorldinputInputSequenceModel::clear() {
   this->beginResetModel();
   this->_clear_silent();
   this->endResetModel();
}
void DKWorldinputInputSequenceModel::overwriteFromSource(const input_sequence& src) {
   this->beginResetModel();

   this->_clear_silent();

   auto clone = [this, &src](const input_sequence::group& g) {
      auto recurse = [&](const input_sequence::group& g, Node* parent, auto& recurse) -> void {
         auto* node = new Node;
         node->type = g.type;
         if (parent) {
            parent->append_child(*node);
         } else {
            assert(!this->_root);
            this->_root = node;
         }

         if (g.type == group_type::single_control) {
            node->button.mouse  = g.button.mouse;
            node->button.vk     = g.button.key.vk;
            node->button.xinput = g.button.gamepad;
         }
         if (&g == src.raycast.associated_button) {
            this->_raycast_associated_button = node;
         }
         if (g.can_have_children()) {
            for (const auto* child : g.children) {
               assert(child != nullptr);
               recurse(*child, node, recurse);
            }
         }
      };
      recurse(g, nullptr, recurse);
   };
   if (src.root)
      clone(*src.root);

   this->endResetModel();
}
void DKWorldinputInputSequenceModel::overwriteDestination(input_sequence& dst) const {
   dst.raycast.associated_button = nullptr;
   if (dst.root) {
      delete dst.root;
      dst.root = nullptr;
   }

   auto clone = [this, &dst](const Node& n) {
      auto recurse = [&](const Node& n, input_sequence::group* parent, auto& recurse) -> void {
         auto* group = new input_sequence::group;
         group->type = n.type;
         if (parent) {
            parent->children.push_back(group);
         } else {
            assert(!dst.root);
            dst.root = group;
         }

         if (n.type == group_type::single_control) {
            group->button = {
               .key     = n.button.vk,
               .mouse   = n.button.mouse,
               .gamepad = n.button.xinput,
            };
         }
         if (&n == this->_raycast_associated_button) {
            dst.raycast.associated_button = group;
         }
         if (n.can_have_children()) {
            for (const auto* child : n.group.children) {
               assert(child != nullptr);
               recurse(*child, group, recurse);
            }
         }
      };
      recurse(n, nullptr, recurse);
   };
   if (this->_root)
      clone(*this->_root);
}

bool DKWorldinputInputSequenceModel::_insertInOrAfter(const QModelIndex& target, Node* item) {
   auto* target_node = this->_node_from_qmi(target);

   if (!target_node) {
      assert(this->_root == nullptr);
      this->beginInsertRows({}, 0, 0);
      this->_root = item;
      this->endInsertRows();
      return true;
   }

   auto* parent_node = target_node;
   int   insert_at   = parent_node->child_count();
   if (!parent_node->can_have_children()) {

      if (parent_node == this->_root) {
         //
         // Special case: the root node is a leaf node. There's no way to insert other 
         // nodes inside of a leaf, nor a parent node to insert a sibling into. Instead, 
         // create a new node to wrap both the root node and the node we wish to insert, 
         // and make this wrapper the new root node.
         //
         this->beginResetModel();
         auto* wrap = new Node;
         wrap->type = group_type::concurrent_ordered;
         wrap->append_child(*this->_root);
         wrap->append_child(*item);
         this->_root = wrap;
         this->endResetModel();
         return true;
      }

      parent_node = parent_node->parent;
      if (!parent_node)
         return false;
      insert_at = parent_node->index_of_child(*target_node) + 1;
   }
   
   auto first_inserted = insert_at;
   auto last_inserted  = insert_at;
   this->beginInsertRows(this->_qmi_for_node(parent_node), first_inserted, last_inserted);
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
   auto* item = new Node();
   item->type = group_type::single_control;
   //
   if (!this->_insertInOrAfter(parent, item)) {
      delete item;
      return {};
   }
   return this->_qmi_for_node(item);
}
std::optional<QModelIndex> DKWorldinputInputSequenceModel::addGroupTo(const QModelIndex& parent) {
   auto* item = new Node();
   item->type = group_type::concurrent_ordered;
   //
   if (!this->_insertInOrAfter(parent, item)) {
      delete item;
      return {};
   }
   return this->_qmi_for_node(item);
}

void DKWorldinputInputSequenceModel::deleteItems(QModelIndexList indices) {
   if (indices.isEmpty())
      return;
   for (const auto& qmi : indices) {
      auto* node = _node_from_qmi(qmi);
      if (!node)
         continue;

      if (node == this->_raycast_associated_button) {
         this->_raycast_associated_button = nullptr;
      }

      if (!node->parent) {
         assert(node == this->_root);
         this->clear();
         return;
      }

      auto parent_qmi = this->_qmi_for_node(node->parent);
      auto i          = node->parent->index_of_child(*node);
      this->beginRemoveRows(parent_qmi, i, i);
      node->parent->remove_child(*node);
      delete node;
      this->endRemoveRows();
   }
}
void DKWorldinputInputSequenceModel::moveItems(QModelIndexList indices, int down) {
   if (indices.isEmpty())
      return;
   for (const auto& index : indices) {
      const auto* node = _node_from_qmi(index);
      if (!node)
         continue;
      const auto* parent = node->parent;
      if (!parent)
         continue;
      auto size = parent->child_count();

      auto i = index.row();
      if (i >= size)
         continue;
      int to = i + down;
      if (to < 0)
         to = 0;
      else if (to >= size)
         to = size - 1;

      if (to == i)
         continue;

      if (down > 0)
         //
         // When moving rows up within the same parent, or across parents, the "destination index" 
         // is the index they will be placed at. However, when moving rows up within the same 
         // parent, the "destination index" is the index that they will be placed BEFORE. In fact, 
         // if you're moving multiple rows, then the "destination index" should be index AFTER the 
         // index that you want the LAST moved element placed at.
         //
         ++to;
      auto parent_qmi = this->parent(index);
      this->moveRow(parent_qmi, i, parent_qmi, to);
   }
}

std::optional<DKWorldinputInputSequenceModel::NodeInfo> DKWorldinputInputSequenceModel::infoFor(const QModelIndex& qmi) const {
   if (qmi.column() != 0) {
      if (_is_empty_qmi(qmi))
         return {};
      return infoFor(qmi.siblingAtColumn(0));
   }
   auto* node = this->_node_from_qmi(qmi);
   if (!node)
      return {};
   return NodeInfo{
      .type   = node->type,
      .button = {
         node->button.mouse,
         node->button.vk,
         node->button.xinput,
      },
   };
}
void DKWorldinputInputSequenceModel::replaceInfoFor(const QModelIndex& qmi, const NodeInfo& info) {
   auto* node = this->_node_from_qmi(qmi);
   if (!node)
      return;

   if (node->type == group_type::single_control) {
      assert(info.type == node->type);
      node->button = {
         info.button.mouse,
         info.button.vk,
         info.button.xinput,
      };
   } else {
      assert(info.type != group_type::single_control);
      node->type = info.type;
   }

   auto parent_qmi = this->parent(qmi);
   auto tl = this->index(qmi.row(), 0, parent_qmi);
   auto br = this->index(qmi.row(), this->columnCount(parent_qmi), parent_qmi);
   this->dataChanged(tl, br);
}

std::optional<QModelIndex> DKWorldinputInputSequenceModel::raycastAssociatedButton() const {
   if (!this->_raycast_associated_button)
      return {};
   return this->_qmi_for_node(this->_raycast_associated_button);
}
bool DKWorldinputInputSequenceModel::isRaycastAssociatedButton(const QModelIndex& qmi) const {
   if (qmi.column() != 0) {
      if (_is_empty_qmi(qmi))
         return this->_raycast_associated_button == nullptr;
      return isRaycastAssociatedButton(qmi.siblingAtColumn(0));
   }
   return qmi == this->_qmi_for_node(this->_raycast_associated_button);
}
bool DKWorldinputInputSequenceModel::setRaycastAssociatedButton(const QModelIndex& qmi) {
   auto* prior = this->_raycast_associated_button;
   auto* node  = this->_node_from_qmi(qmi);
   if (!node) {
      this->_raycast_associated_button = nullptr;
      if (prior) {
         const QModelIndex qmi_prior = this->_qmi_for_node(this->_raycast_associated_button, Columns::RaycastAssociatedIndicator);
         emit dataChanged(qmi_prior, qmi_prior);
      }
      return true;
   }
   if (node->type != group_type::single_control)
      return false;
   this->_raycast_associated_button = node;
   if (prior) {
      const QModelIndex qmi_prior = this->_qmi_for_node(this->_raycast_associated_button, Columns::RaycastAssociatedIndicator);
      emit dataChanged(qmi_prior, qmi_prior);
   }
   if (node) {
      auto qmi_ra = qmi.siblingAtColumn(Columns::RaycastAssociatedIndicator);
      emit dataChanged(qmi_ra, qmi_ra);
   }
   return true;
}