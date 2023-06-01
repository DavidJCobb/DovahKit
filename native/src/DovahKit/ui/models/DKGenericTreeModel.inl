#pragma once
#include "./DKGenericTreeModel.h"
#include <QItemSelection>

#pragma push_macro("CLASS_TEMPLATE_PARAMS")
#pragma push_macro("CLASS_NAME")

#pragma region DKGenericTreeModelNodeBase
   #define CLASS_TEMPLATE_PARAMS template<class Model>
   #define CLASS_NAME DKGenericTreeModelNodeBase<Model>

   CLASS_TEMPLATE_PARAMS
   CLASS_NAME::~DKGenericTreeModelNodeBase() {
      for (auto* child : this->_children) {
         child->_parent = nullptr;
         delete child;
      }
      this->_children.clear();
      if (this->_parent) {
         this->_parent->remove_child(*(node_type*)this);
         this->_parent = nullptr;
      }
   }

   CLASS_TEMPLATE_PARAMS
   size_t CLASS_NAME::child_count() const noexcept {
      return this->_children.size();
   }

   CLASS_TEMPLATE_PARAMS
   bool CLASS_NAME::contains(const node_type& subject) const noexcept {
      if (&subject == this)
         return false;
      for (const auto* child : this->_children)
         if (child->contains(subject))
            return true;
      return false;
   }

   CLASS_TEMPLATE_PARAMS
   size_t CLASS_NAME::index_of_child(const node_type& child) const noexcept {
      if (child._parent != this)
         return -1;
      return this->_children.indexOf((node_type*)&child);
   }

   CLASS_TEMPLATE_PARAMS
   const typename CLASS_NAME::node_type* CLASS_NAME::nth_child(size_t n) const noexcept {
      return this->_children[n];
   }

   CLASS_TEMPLATE_PARAMS
   const typename CLASS_NAME::node_type* CLASS_NAME::parent_node() const noexcept {
      return this->_parent;
   }

   CLASS_TEMPLATE_PARAMS
   void CLASS_NAME::move_children(size_t start, size_t count, size_t destination_start) {
      assert(this->_check_can_have_children());
      if (!count)
         return;
      assert(start >= 0);
      assert(start + count <= this->child_count());
      assert(destination_start >= 0);
      assert(destination_start < this->child_count());
      while (count--)
         this->_children.move(start + count, destination_start);
   }

   CLASS_TEMPLATE_PARAMS
   void CLASS_NAME::move_children_to(size_t start, size_t count, node_type& destination_parent, size_t destination_start) {
      assert(this->_check_can_have_children());
      if (!count)
         return;
      assert(start >= 0);
      assert(start + count <= this->child_count());
      assert(destination_start >= 0);
      assert(destination_start < destination_parent.child_count());
      destination_parent._children.reserve(destination_parent.child_count() + count);
      for (size_t i = 0; i < count; ++i) {
         auto* child = this->_children[start + i];
         child->_parent = &destination_parent;
         destination_parent._children.insert(destination_start + i, child);
      }
      this->_children.remove(start, count);
   }

   CLASS_TEMPLATE_PARAMS
   void CLASS_NAME::append_child(node_type& child) {
      assert(this->_check_can_have_children());
      if (child._parent) {
         if (child._parent == this)
            return;
         child._parent->remove_child(child);
      }
      child._parent = (node_type*)this;
      this->_children.append(&child);
   }

   CLASS_TEMPLATE_PARAMS
   void CLASS_NAME::insert_child(size_t at, node_type& child) {
      assert(this->_check_can_have_children());
      if (child._parent) {
         if (child._parent == this)
            return;
         child._parent->remove_child(child);
      }
      child._parent = (node_type*)this;
      this->_children.insert(at, &child);
   }

   CLASS_TEMPLATE_PARAMS
   void CLASS_NAME::remove_child(node_type& child) {
      assert(this->_check_can_have_children());
      assert(child._parent == this);
      child._parent = nullptr;
      bool result = this->_children.removeOne(&child);
      assert(result);
   }

   #undef CLASS_TEMPLATE_PARAMS
   #undef CLASS_NAME
#pragma endregion

#define CLASS_TEMPLATE_PARAMS template<class Self>
#define CLASS_NAME DKGenericTreeModel<Self>

CLASS_TEMPLATE_PARAMS
CLASS_NAME::~DKGenericTreeModel() {
   this->clear();
}

CLASS_TEMPLATE_PARAMS
constexpr bool CLASS_NAME::qmi_is_mine(const QModelIndex& qmi) const noexcept {
   return qmi.model() == this;
}

CLASS_TEMPLATE_PARAMS
constexpr bool CLASS_NAME::qmi_is_none(const QModelIndex& qmi) const noexcept {
   return !qmi.isValid();
}

CLASS_TEMPLATE_PARAMS
const typename CLASS_NAME::node_type* CLASS_NAME::node_for_qmi(const QModelIndex& qmi) const {
   if (!qmi.isValid())
      return &this->invisible_root;
   if (!qmi_is_mine(qmi))
      return nullptr;
   return (const node_type*)qmi.internalPointer();
}

CLASS_TEMPLATE_PARAMS
QModelIndex CLASS_NAME::qmi_for_node(const node_type& node, size_t column) const {
   if (&node == &this->invisible_root)
      return {};
   const node_type* parent = node.parent_node();
   assert(parent != nullptr);
   return this->createIndex(parent->index_of_child(node), column, (void*)&node);
}

#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      CLASS_TEMPLATE_PARAMS
      QModelIndex CLASS_NAME::index(int row, int column, const QModelIndex& parent) const {
         if (row < 0 || column < 0)
            return {};
         if (column >= self_type::max_columns)
            return {};

         const node_type* parent_node = node_for_qmi(parent);
         if (!parent_node)
            return {};
         if (row >= parent_node->child_count())
            return {};
         if constexpr (self_type::variable_column_count) {
            if (column >= this->column_count_of(parent_node)) {
               return {};
            }
         }

         return this->createIndex(row, column, (void*)parent_node->nth_child(row));
      }

      CLASS_TEMPLATE_PARAMS
      QModelIndex CLASS_NAME::parent(const QModelIndex& index) const {
         if (!index.isValid())
            return {};
         auto* node = this->node(index);
         if (!node)
            return {};
         return this->index(node->parent_node());
      }

      CLASS_TEMPLATE_PARAMS
      QModelIndex CLASS_NAME::sibling(int row, int column, const QModelIndex& index) const {
         if (row < 0 || column < 0)
            return {};
         if (column > self_type::max_columns)
            return {};
         if constexpr (!self_type::variable_column_count) {
            if (row == index.row()) {
               return this->createIndex(row, column, index.internalPointer());
            }
         }
         const node_type* node = node_for_qmi(index);
         if (!node)
            return {};
         const node_type* parent = node->parent_node();
         if (!parent)
            return {};
         if (row >= parent->child_count())
            return {};
         const node_type* child = parent->nth_child(row);
         if constexpr (self_type::variable_column_count) {
            if (column >= ((const self_type*)this)->column_count_of(child))
               return {};
         }
         return this->createIndex(row, column, (void*)child);
      }

      CLASS_TEMPLATE_PARAMS
      int CLASS_NAME::rowCount(const QModelIndex& qmi) const {
         if (qmi.column() > 0)
            return 0;
         const node_type* node = node_for_qmi(qmi);
         if (!node)
            return 0;
         return node->child_count();
      }

      CLASS_TEMPLATE_PARAMS
      int CLASS_NAME::columnCount(const QModelIndex& qmi) const {
         if constexpr (self_type::variable_column_count) {
            const node_type* node = node_for_qmi(qmi);
            if (!node)
               return 0;
            return ((const self_type*)this)->column_count_of(node);
         } else {
            return self_type::max_columns;
         }
      }

      CLASS_TEMPLATE_PARAMS
      bool CLASS_NAME::moveRows(const QModelIndex& from_parent, int first_row_index, int count, const QModelIndex& to_parent, int to_position) {
         //
         // When moving rows from one parent to another, the signature is:
         // 
         //    from_parent, first_row_index, count, to_parent, place_at
         // 
         // When moving rows within the same parent, however, the signature is:
         // 
         //    from_parent, first_row_index, count, to_parent, place_last_moved_before
         // 
         // The API is designed around the assumption that same-parent moves will be done 
         // thusly (assuming QVector):
         // 
         //    auto slice = children.mid(first_row_index, count);
         //    children.remove(first_row_index, count);
         //    for(int i = slice.size() - 1; i >= 0; --i)
         //       children.insert(place_last_moved_before - count, slice[i]);
         // 
         // This API is fucking awful, which is probably why Qt themselves almost never 
         // use it. It's not even implemented in QStandardItemModel.
         //
         if (first_row_index < 0 || to_position < 0)
            return false;
         if (count <= 0)
            return false;
         if (!from_parent.isValid() || !to_parent.isValid())
            return false;

         auto* src_parent = node_for_qmi(from_parent);
         auto* dst_parent = node_for_qmi(to_parent);
         assert(src_parent != nullptr);
         assert(dst_parent != nullptr);

         if (first_row_index + count > src_parent->child_count())
            return false;

         auto last_to_move = first_row_index + count - 1;
         if (src_parent != dst_parent) {
            if (to_position > dst_parent->child_count()) // don't move past the end of the destination parent
               return false;
         } else {
            bool moving_down = to_position > first_row_index;

            if (first_row_index == to_position)
               return false;
            if (to_position > src_parent->child_count())
               return false;
            if (moving_down) {
               if (to_position - count < 0)
                  return false;
            } else {
               if (to_position < 0)
                  return false;
            }
         }

         if (!beginMoveRows(from_parent, first_row_index, last_to_move, to_parent, to_position))
            return false;

         if (src_parent != dst_parent) {
            src_parent->move_children_to(first_row_index, count, *dst_parent, to_position);
         } else {
            bool moving_down = to_position > first_row_index;
            if (moving_down) {
               src_parent->move_children(first_row_index, count, to_position - count);
            } else {
               src_parent->move_children(first_row_index, count, to_position);
            }
         }

         endMoveRows();
         return true;
      }
   #pragma endregion
   #pragma region Node data
      CLASS_TEMPLATE_PARAMS
      QVariant CLASS_NAME::data(const QModelIndex& index, int role) const {
         if (!index.isValid())
            return {};
         const auto* node = node_for_qmi(index);
         if (!node)
            return {};
         return ((const self_type*)this)->data_of(*node, (Qt::ItemDataRole)role, index.column());
      }

      CLASS_TEMPLATE_PARAMS
      Qt::ItemFlags CLASS_NAME::flags(const QModelIndex& index) const {
         if (!index.isValid())
            return {};
         const auto* node = node_for_qmi(index);
         if (!node)
            return {};
         return ((const self_type*)this)->flags_of(*node, index.column());
      }
   #pragma endregion
#pragma endregion
      
CLASS_TEMPLATE_PARAMS
void CLASS_NAME::_clear_silent() {
   //
   // We want to delete all child nodes of the invisible root. A child node's destructor 
   // will attempt to remove the child node from the parent's child list; as such, if we 
   // iterate directly on the child list, our iterator will be invalidated out from under 
   // us. Instead, copy the list and use that.
   //
   auto list = this->invisible_root._children;
   for (auto* node : list) {
      ((self_type*)this)->on_before_delete_node(*node);
      delete node;
   }
}

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
      if (!qmi.isValid() || !qmi_is_mine(qmi))
         continue;
      targets.push_back(node_for_qmi(qmi));
   }

   if (targets.size() > 1) {
      //
      // If `qml_list` includes multiple nodes, and one of those nodes contains any of 
      // the others, then we need to filter the list in order to avoid any accidental 
      // double-frees.
      //
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
            if (a == b || a->contains(*b)) {
               targets[j] = nullptr;
            }
         }
      }
   }
   for (auto* target : targets) {
      if (!target) // Pointer was nulled out above to prevent double-delete
         continue;

      auto* parent = target->parent_node();
      assert(parent != nullptr);
      auto  index  = parent->index_of_child(*target);

      this->beginRemoveRows(this->index(parent), index, index);
      ((self_type*)this)->on_before_delete_node(*target);
      delete target;
      this->endRemoveRows();
   }
}

CLASS_TEMPLATE_PARAMS
void CLASS_NAME::moveItem(const QModelIndex& qmi, int down) {
   if (!qmi.isValid() || !qmi_is_mine(qmi))
      return;
   
   int disgusting_same_parent_move_hack = (down > 0) ? 1 : 0;

   auto parent_qmi = this->parent(qmi);
   this->moveRows(parent_qmi, qmi.row(), 1, parent_qmi, qmi.row() + down + disgusting_same_parent_move_hack);
}

CLASS_TEMPLATE_PARAMS
void CLASS_NAME::moveItems(const QModelIndex& start, const QModelIndex& end, int down) {
   if (!start.isValid() || !qmi_is_mine(start))
      return;
   if (!end.isValid() || !qmi_is_mine(end))
      return;
   if (start.internalPointer() != end.internalPointer()) // require same parent
      return;
   if (start.row() > end.row())
      return;

   int count = end.row() - start.row() + 1;

   int disgusting_same_parent_move_hack = (down > 0) ? 1 : 0;
   
   auto parent_qmi = this->parent(start);
   this->moveRows(parent_qmi, start.row(), count, parent_qmi, start.row() + down + disgusting_same_parent_move_hack);
}

CLASS_TEMPLATE_PARAMS
void CLASS_NAME::moveItems(const QItemSelection& ranges, int down) {
   bool discontiguous_within_one_parent = false;
   if (ranges.size() > 1) {
      QVector<node_type*> parents;
      for (const auto& range : ranges) {
         parents.push_back(this->node(range.topLeft())->parent_node());
      }
      for (size_t i = 0; i < parents.size(); ++i) {
         for (size_t j = i + 1; j < parents.size(); ++j) {
            if (parents[i] == parents[j]) {
               discontiguous_within_one_parent = true;
               break;
            }
         }
      }
   }
   if (discontiguous_within_one_parent) {
      //
      // The only way to avoid breaking things in this case is to use QPersistentModelIndexes.
      //
      using persistent_range = QPair<QPersistentModelIndex, QPersistentModelIndex>;
      QVector<persistent_range> qpmi;
      for (const auto& range : ranges) {
         qpmi.push_back(range.topLeft(), range.bottomRight());
      }
      for (const auto& range : qpmi) {
         this->moveItems(range.topLeft(), range.bottomRight(), down);
      }
      return;
   }

   for (const auto& range : ranges) {
      this->moveItems(range.topLeft(), range.bottomRight(), down);
   }
}

CLASS_TEMPLATE_PARAMS
const typename CLASS_NAME::node_type* CLASS_NAME::node(const QModelIndex& qmi) const {
   return this->node_for_qmi(qmi);
}

CLASS_TEMPLATE_PARAMS
const typename CLASS_NAME::node_type* CLASS_NAME::parentNode(const QModelIndex& qmi) const {
   return this->parent_node_for_qmi(qmi);
}

CLASS_TEMPLATE_PARAMS
QModelIndex CLASS_NAME::index(const node_type* node, size_t column) const {
   if (!node)
      return {};
   return this->qmi_for_node(*node, column);
}

#pragma region Subclass helpers
   CLASS_TEMPLATE_PARAMS
   void CLASS_NAME::emitNodeChanged(const node_type& node) {
      auto* parent = node.parent_node();
      int i = 0;
      if (parent) {
         i = parent->index_of_child(node);
      }

      QModelIndex start = this->createIndex(i, 0, (void*)parent);
      QModelIndex end;
      if constexpr (self_type::variable_column_count) {
         end = this->createIndex(i, ((const self_type*)this)->column_count_of(node), parent);
      } else {
         end = this->createIndex(i, self_type::max_columns, (void*)parent);
      }
      emit dataChanged(start, end);
   }

   CLASS_TEMPLATE_PARAMS
   void CLASS_NAME::emitNodeChanged(const node_type& node, size_t column) {
      auto* parent = node.parent_node();
      int i = 0;
      if (parent) {
         i = parent->index_of_child(node);
      }

      QModelIndex qmi = this->createIndex(i, column, (void*)parent);
      emit dataChanged(qmi, qmi);
   }

   CLASS_TEMPLATE_PARAMS
   void CLASS_NAME::emitNodeChanged(const QModelIndex& qmi) {
      QModelIndex start = this->sibling(qmi.row(), 0, qmi);
      QModelIndex end;
      if constexpr (self_type::variable_column_count) {
         end = this->sibling(qmi.row(), this->columnCount(qmi), qmi);
      } else {
         end = this->sibling(qmi.row(), self_type::max_columns, qmi);
      }
      emit dataChanged(start, end);
   }

   CLASS_TEMPLATE_PARAMS
   void CLASS_NAME::replaceAll(node_type* root) {
      this->beginResetModel();
      this->_clear_silent();
      if (root)
         this->invisible_root.append_child(*root);
      this->endResetModel();
   }
#pragma endregion

#undef CLASS_TEMPLATE_PARAMS
#undef CLASS_NAME

#pragma pop_macro("CLASS_TEMPLATE_PARAMS")
#pragma pop_macro("CLASS_NAME")