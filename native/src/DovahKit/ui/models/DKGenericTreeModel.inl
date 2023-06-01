#pragma once
#include "./DKGenericTreeModel.h"
#include <QItemSelection>

#pragma push_macro("CLASS_TEMPLATE_PARAMS")
#pragma push_macro("CLASS_NAME")
#define CLASS_TEMPLATE_PARAMS template<typename Self, DKGenericTreeModelNode Node>
#define CLASS_NAME DKGenericTreeModel<Self, Node>

CLASS_TEMPLATE_PARAMS
constexpr bool CLASS_NAME::qmi_is_mine(const QModelIndex& qmi) const noexcept {
   return qmi.model() == this;
}

CLASS_TEMPLATE_PARAMS
constexpr bool CLASS_NAME::qmi_is_none(const QModelIndex& qmi) const noexcept {
   return (qmi.row() < 0 || qmi.column() < 0);
}

CLASS_TEMPLATE_PARAMS
constexpr bool CLASS_NAME::qmi_is_root(const QModelIndex& qmi) const noexcept {
   return qmi_is_mine(qmi) && !qmi_is_none(qmi) && qmi.row() == 0 && qmi.internalPointer() == nullptr;
}

CLASS_TEMPLATE_PARAMS
constexpr QModelIndex CLASS_NAME::make_qmi_for_root(size_t column) const noexcept {
   if (!this->_root)
      return {};
   return QModelIndex(0, column, nullptr, this);
}

CLASS_TEMPLATE_PARAMS
const CLASS_NAME::node_type* CLASS_NAME::parent_node_for_qmi(const QModelIndex& qmi) const {
   if (!qmi_is_mine(qmi))
      return nullptr;
   return ((const node_type*)qmi.internalPointer());
}

CLASS_TEMPLATE_PARAMS
CLASS_NAME::node_type* CLASS_NAME::parent_node_for_qmi(const QModelIndex& qmi) {
   return const_cast<node_type*>(std::as_const(*this).parent_node_for_qmi(qmi));
}

CLASS_TEMPLATE_PARAMS
const CLASS_NAME::node_type* CLASS_NAME::node_for_qmi(const QModelIndex& qmi) const {
   const node_type* parent = parent_node_for_qmi(qmi);
   if (!parent) {
      if (qmi_is_root(qmi))
         return this->_root;
      return nullptr;
   }
   if (qmi.row() >= parent->child_count())
      return nullptr;
   return parent->nth_child(qmi.row());
}

CLASS_TEMPLATE_PARAMS
CLASS_NAME::node_type* CLASS_NAME::node_for_qmi(const QModelIndex& qmi) {
   return const_cast<node_type*>(std::as_const(*this).parent_node_for_qmi(qmi));
}

CLASS_TEMPLATE_PARAMS
QModelIndex CLASS_NAME::qmi_for_node(const node_type& node, size_t column) const {
   if (&node == this->_root) {
      return QModelIndex(0, column, nullptr, *this);
   }
   const node_type* parent = node.parent_node();
   assert(parent != nullptr);
   return QModelIndex(parent->index_of_child(node), column, parent, this);
}

#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      CLASS_TEMPLATE_PARAMS
      QModelIndex CLASS_NAME::index(int row, int column, const QModelIndex& parent) const {
         if (!this->_root || row < 0 || column < 0)
            return {};
         if (column > self_type::max_columns)
            return {};

         const node_type* parent_node = node_for_qmi(parent);
         if (!parent_node) {
            if (row == 0)
               return make_qmi_for_root(column);
            return {};
         }
         if (row >= parent_node->child_count())
            return {};
         if constexpr (self_type::variable_column_count) {
            if (column >= this->column_count_of(parent_node)) {
               return {};
            }
         }

         return this->createIndex(row, column, (void*)parent_node);
      }

      CLASS_TEMPLATE_PARAMS
      QModelIndex CLASS_NAME::parent(const QModelIndex& index) const {
         const node_type* parent = parent_node_for_qmi(index);
         if (!parent || parent == this->_root)
            return {};
         return qmi_for_node(parent, index.column());
      }

      CLASS_TEMPLATE_PARAMS
      QModelIndex CLASS_NAME::sibling(int row, int column, const QModelIndex& index) const {
         if (row < 0 || column < 0)
            return {};
         if (column > self_type::max_columns)
            return {};
         if constexpr (!self_type::variable_column_count) {
            if (row == index.row()) {
               return QModelIndex(row, column, index.internalPointer(), this);
            }
         }
         const node_type* parent = parent_node_for_qmi(index);
         if (!parent) {
            if (row != 0)
               return {};
            return make_qmi_for_root(column);
         }
         if (row >= parent->child_count())
            return {};
         const node_type* child = parent->nth_child(row);
         if constexpr (self_type::variable_column_count) {
            if (column >= ((const self_type*)this)->column_count_of(child))
               return {};
         }
         return QModelIndex(row, column, parent, this);
      }

      CLASS_TEMPLATE_PARAMS
      int CLASS_NAME::rowCount(const QModelIndex& qmi) const {
         if (qmi_is_root(qmi)) {
            return this->_root->child_count();
         }
         if (qmi_is_none(qmi)) {
            return 1; // our root
         }
         const node_type* node = node_for_qmi(qmi);
         assert(node != nullptr);
         return node->child_count();
      }

      CLASS_TEMPLATE_PARAMS
      int CLASS_NAME::columnCount(const QModelIndex& qmi) const {
         if constexpr (self_type::variable_column_count) {
            const node_type* node = node_for_qmi(qmi);
            if (!node)
               return -1;
            return ((const self_type*)this)->column_count_of(node);
         } else {
            return MaxColumns;
         }
      }

      CLASS_TEMPLATE_PARAMS
      bool CLASS_NAME::moveRows(const QModelIndex& from_parent, int first_row_index, int count, const QModelIndex& to_parent, int to_position) {
         //
         // When moving rows from one parent to another, the signature is:
         // 
         //    from_parent, first_row_index, count, to_parent, place_at
         // 
         // When moving rows within the same parent, the signature is:
         // 
         //    from_parent, first_row_index, count, to_parent, place_last_moved_before
         //
         if (first_row_index < 0 || to_position < 0)
            return false;
         if (count <= 0)
            return false;

         if (qmi_is_none(from_parent) || qmi_is_none(to_parent)) // don't allow moving the root / don't allow giving the root a sibling
            return false;

         auto* src_parent = node_for_qmi(from_parent);
         auto* dst_parent = node_for_qmi(to_parent);
         assert(src_parent != nullptr);
         assert(dst_parent != nullptr);
         if (src_parent->contains(dst_parent)) // don't allow moving a node inside of its own descendants
            return false;

         if (first_row_index + count > src_parent->child_count())
            return false;

         if (src_parent != dst_parent) {
            if (to_position > dst_parent->child_count()) // don't move past the end of the destination parent
               return false;
            src_parent->move_children_to(first_row_index, count, *dst_parent, to_position);
         } else {
            if (first_row_index == to_position)
               return false;
            if (to_position - count < 0)
               return false;
            if (to_position >= src_parent->child_count())
               return false;
         }

         auto last_to_move = first_row_index + count - 1;

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
            src_parent->move_children_to(first_row_index, count, *dst_parent, to_position);
         } else {
            src_parent->move_children_to(first_row_index, count, to_position - count);
         }

         endMoveRows();
         return true;
      }
   #pragma endregion
   #pragma region Node data
      CLASS_TEMPLATE_PARAMS
      QVariant CLASS_NAME::data(const QModelIndex& index, int role) const {
         const auto* node = node_for_qmi(index);
         if (!node)
            return {};
         return ((const self_type*)this)->data_of(*node, role, index.column());
      }

      CLASS_TEMPLATE_PARAMS
      Qt::ItemFlags CLASS_NAME::flags(const QModelIndex& index) const {
         const auto* node = node_for_qmi(index);
         if (!node)
            return {};
         return ((const self_type*)this)->flags_of(*node, index.column());
      }
   #pragma endregion
#pragma endregion

CLASS_TEMPLATE_PARAMS
void CLASS_NAME::clear() {
   this->beginResetModel();
   this->_clear_silent();
   this->endResetModel();
}

CLASS_TEMPLATE_PARAMS
void CLASS_NAME::deleteItems(QModelIndexList qmi_list) {
   if (qmi_list.isEmpty())
      return;
   std::vector<node_type*> targets;
   for (const auto& qmi : qmi_list) {
      if (qmi_is_none(qmi) || !qmi_is_mine(qmi))
         continue;
      if (qmi_is_root(qmi)) {
         this->clear();
         return;
      }
      targets.push_back(node_for_qmi(qmi));
   }

   for (size_t i = 0; i < targets.size(); ++i) {
      auto* a = targets[i];
      if (!a)
         continue;
      for (size_t j = 0; j < targets.size(); ++j) {
         if (j == i)
            continue;
         auto* b = targets[j];
         if (!b)
            continue;
         if (a == b || a->contains(b)) {
            targets[j] = nullptr;
         }
      }
   }
   for (auto* target : targets)
      delete target;
}

CLASS_TEMPLATE_PARAMS
void CLASS_NAME::moveItem(const QModelIndex& qmi, int down) {
   if (qmi_is_none(qmi) || qmi_is_root(qmi) || !qmi_is_mine(qmi))
      return;
   
   auto parent_qmi = this->parent(qmi);
   this->moveRows(parent_qmi, start.row(), 1, parent_qmi, start.row() + down);
}

CLASS_TEMPLATE_PARAMS
void CLASS_NAME::moveItems(const QModelIndex& start, const QModelIndex& end, int down) {
   if (qmi_is_none(start) || qmi_is_root(start) || !qmi_is_mine(start))
      return;
   if (qmi_is_none(end) || qmi_is_root(end) || !qmi_is_mine(end))
      return;
   if (start.internalPointer() != end.internalPointer()) // require same parent
      return;
   if (start.row() > end.row())
      return;

   int count = end.row() - start.row() + 1;
   
   auto parent_qmi = this->parent(start);
   this->moveRows(parent_qmi, start.row(), count, parent_qmi, start.row() + down);
}

CLASS_TEMPLATE_PARAMS
void CLASS_NAME::moveItems(const QItemSelection& ranges, int down) {
   for (const auto& range : ranges) {
      this->moveItems(range.topLeft(), range.bottomRight(), down);
   }
}

CLASS_TEMPLATE_PARAMS
const CLASS_NAME::node_type* CLASS_NAME::node(const QModelIndex& qmi) const {
   return this->node_for_qmi(qmi);
}

CLASS_TEMPLATE_PARAMS
QModelIndex CLASS_NAME::index(const node_type* node) const {
   return this->qmi_for_node(node);
}

#undef CLASS_TEMPLATE_PARAMS
#undef CLASS_NAME
#pragma pop_macro("CLASS_TEMPLATE_PARAMS")
#pragma pop_macro("CLASS_NAME")