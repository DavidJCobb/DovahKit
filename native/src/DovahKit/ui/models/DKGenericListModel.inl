#pragma once
#include "./DKGenericListModel.h"
#include <QItemSelection>

#pragma push_macro("CLASS_TEMPLATE_PARAMS")
#pragma push_macro("CLASS_NAME")

#define CLASS_TEMPLATE_PARAMS template<class Self, typename Node>
#define CLASS_NAME DKGenericListModel<Self, Node>

CLASS_TEMPLATE_PARAMS
CLASS_NAME::~DKGenericListModel() {
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
      return nullptr;
   if (!qmi_is_mine(qmi))
      return nullptr;
   return (const node_type*)qmi.internalPointer();
}

CLASS_TEMPLATE_PARAMS
QModelIndex CLASS_NAME::qmi_for_node(const node_type& node, size_t column) const {
   auto i = this->_nodes.indexOf(const_cast<node_type*>(&node)); // fuck it
   if (i < 0)
      return {};
   return this->createIndex(i, column, (void*)&node);
}

#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      CLASS_TEMPLATE_PARAMS
      QModelIndex CLASS_NAME::index(int row, int column, const QModelIndex& parent) const {
         if (row < 0 || column < 0)
            return {};
         if (column >= self_type::column_count)
            return {};
         if (row >= this->rowCount())
            return {};
         if (parent.isValid())
            return {};
         return this->createIndex(row, column, (void*)this->_nodes[row]);
      }

      CLASS_TEMPLATE_PARAMS
      QModelIndex CLASS_NAME::parent(const QModelIndex& index) const {
         return {};
      }

      CLASS_TEMPLATE_PARAMS
      QModelIndex CLASS_NAME::sibling(int row, int column, const QModelIndex& index) const {
         if (row < 0 || column < 0)
            return {};
         if (column >= self_type::column_count)
            return {};
         if (row >= this->rowCount())
            return {};
         const node_type* node = node_for_qmi(index);
         if (!node)
            return {};
         const node_type* child = this->_nodes[row];
         return this->createIndex(row, column, (void*)child);
      }

      CLASS_TEMPLATE_PARAMS
      int CLASS_NAME::rowCount(const QModelIndex& qmi) const {
         if (qmi.isValid())
            return 0;
         return this->_nodes.size();
      }
      
      CLASS_TEMPLATE_PARAMS
      int CLASS_NAME::rowCount() const {
         return this->_nodes.size();
      }

      CLASS_TEMPLATE_PARAMS
      int CLASS_NAME::columnCount(const QModelIndex& qmi) const {
         if (qmi.isValid())
            return 0;
         return self_type::column_count;
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
         // This API is awful, which is probably why Qt themselves almost never use it. 
         // It's not even implemented in QStandardItemModel.
         //
         if (first_row_index < 0 || to_position < 0)
            return false;
         if (count <= 0)
            return false;
         if (from_parent.isValid() || to_parent.isValid())
            return false;

         if (first_row_index + count > this->rowCount())
            return false;

         bool moving_down  = to_position > first_row_index;
         auto last_to_move = first_row_index + count - 1;
         {

            if (first_row_index == to_position)
               return false;
            if (to_position > this->rowCount())
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

         {
            auto slice = this->_nodes.mid(first_row_index, count);
            this->_nodes.remove(first_row_index, count);
            if (moving_down) {
               for (int i = slice.size() - 1; i >= 0; --i)
                  this->_nodes.insert(to_position - count, slice[i]);
            } else {
               for (size_t i = 0; i < slice.size(); ++i)
                  this->_nodes.insert(to_position + i, slice[i]);
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
         if (!index.isValid()) {
            if (((const self_type*)this)->allow_inbound_drag_and_drop())
               return Qt::ItemFlag::ItemIsDropEnabled;
            return {};
         }
         const auto* node = node_for_qmi(index);
         if (!node)
            return {};
         return ((const self_type*)this)->flags_of(*node, index.column());
      }
   #pragma endregion
#pragma endregion
      
CLASS_TEMPLATE_PARAMS
void CLASS_NAME::_clear_silent() {
   for (auto* node : this->_nodes) {
      ((self_type*)this)->on_before_delete_node(*node);
      delete node;
   }
   this->_nodes.clear();
}

CLASS_TEMPLATE_PARAMS
void CLASS_NAME::clear() {
   this->beginResetModel();
   this->_clear_silent();
   this->endResetModel();
}

CLASS_TEMPLATE_PARAMS
bool CLASS_NAME::deleteItems(int at, int count) {
   if (count < 1 || at < 0 || at + count > this->rowCount())
      return false;
   this->beginRemoveRows({}, at, at + count - 1);
   for (size_t i = 0; i < count; ++i) {
      delete this->_nodes[at + i];
   }
   this->_nodes.erase(this->_nodes.begin() + at, this->_nodes.begin() + at + count);
   this->endRemoveRows();
   return true;
}

CLASS_TEMPLATE_PARAMS
void CLASS_NAME::moveItem(const QModelIndex& qmi, int down) {
   if (!qmi.isValid() || !qmi_is_mine(qmi))
      return;
   int disgusting_same_parent_move_hack = (down > 0) ? 1 : 0;
   this->moveRows({}, qmi.row(), 1, {}, qmi.row() + down + disgusting_same_parent_move_hack);
}

CLASS_TEMPLATE_PARAMS
void CLASS_NAME::moveItems(const QModelIndex& start, const QModelIndex& end, int down) {
   if (!start.isValid() || !qmi_is_mine(start))
      return;
   if (!end.isValid() || !qmi_is_mine(end))
      return;
   if (start.row() > end.row())
      return;

   int count = end.row() - start.row() + 1;

   int disgusting_same_parent_move_hack = (down > 0) ? 1 : 0;
   this->moveRows({}, start.row(), count, {}, start.row() + down + disgusting_same_parent_move_hack);
}

CLASS_TEMPLATE_PARAMS
void CLASS_NAME::moveItems(const QItemSelection& ranges, int down) {
   bool discontiguous_within_one_parent = ranges.size() > 1;
   if (discontiguous_within_one_parent) {
      //
      // The only way to avoid breaking things in this case is to use QPersistentModelIndexes.
      //
      using persistent_range = QPair<QPersistentModelIndex, QPersistentModelIndex>;
      QVector<persistent_range> qpmi;
      for (const auto& range : ranges) {
         qpmi.push_back({ range.topLeft(), range.bottomRight() });
      }
      for (const auto& range : qpmi) {
         this->moveItems(range.first, range.second, down);
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
QModelIndex CLASS_NAME::index(const node_type* node, size_t column) const {
   if (!node)
      return {};
   return this->qmi_for_node(*node, column);
}

#pragma region Subclass helpers
   CLASS_TEMPLATE_PARAMS
   void CLASS_NAME::emitNodeChanged(const node_type& node) {
      QModelIndex start = this->qmi_for_node(node, 0);
      QModelIndex end   = start.siblingAtColumn(self_type::column_count);
      emit dataChanged(start, end);
   }

   CLASS_TEMPLATE_PARAMS
   void CLASS_NAME::emitNodeChanged(const node_type& node, size_t column) {
      QModelIndex qmi = this->qmi_for_node(node, column);
      emit dataChanged(qmi, qmi);
   }

   CLASS_TEMPLATE_PARAMS
   void CLASS_NAME::emitNodeChanged(const QModelIndex& qmi) {
      QModelIndex start = this->sibling(qmi.row(), 0, qmi);
      QModelIndex end   = this->sibling(qmi.row(), self_type::column_count, qmi);
      emit dataChanged(start, end);
   }
#pragma endregion

#undef CLASS_TEMPLATE_PARAMS
#undef CLASS_NAME

#pragma pop_macro("CLASS_TEMPLATE_PARAMS")
#pragma pop_macro("CLASS_NAME")