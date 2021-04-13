#include "lua_item_model.h"

#pragma region DovahKitLuaCompatibleItem
void DovahKitLuaCompatibleItem::_addColumns(int at, int count) {
   if (!count)
      return;
   int br = this->rowCount();
   int bc = this->columnCount();
   assert(at <= bc);
   int bs = this->_children.size();
   int ac = bc + count;
   //
   QVector<DovahKitLuaCompatibleItem*> rearranged;
   rearranged.resize(br * ac);
   for (int i = 0; i < bs; ++i) {
      int r = i / bc;
      int c = i % bc;
      if (c >= at)
         c += count;
      rearranged[r * ac + c] = this->_children[i];
   }
   this->_children = rearranged;
   this->_cols += count;
}
void DovahKitLuaCompatibleItem::_addRows(int at, int count) {
   if (!count)
      return;
   int br = this->rowCount();
   int bc = this->columnCount();
   assert(at <= br);
   int bs = this->_children.size();
   int ar = br + count;
   //
   QVector<DovahKitLuaCompatibleItem*> rearranged;
   rearranged.resize(ar * bc);
   for (int i = 0; i < bs; ++i) {
      int r = i / bc;
      int c = i % bc;
      if (r >= at)
         r += count;
      rearranged[r * bc + c] = this->_children[i];
   }
   this->_children = rearranged;
   this->_rows += count;
}
void DovahKitLuaCompatibleItem::_deleteColumns(int at, int count) {
   if (!count)
      return;
   int br = this->rowCount();
   int bc = this->columnCount();
   assert(count <= bc);
   assert(at <= bc);
   int bs = this->_children.size();
   int ac = bc - count;
   //
   QVector<DovahKitLuaCompatibleItem*> rearranged;
   rearranged.resize(br * ac);
   for (int i = 0; i < bs; ++i) {
      auto* item = this->_children[i];
      //
      int r = i / bc;
      int c = i % bc;
      if (c >= at) {
         c -= count;
         if (c < 0) {
            if (item)
               delete item;
            continue;
         }
      }
      rearranged[r * ac + c] = item;
   }
   this->_children = rearranged;
   this->_cols -= count;
}
void DovahKitLuaCompatibleItem::_deleteRows(int at, int count) {
   if (!count)
      return;
   int br = this->rowCount();
   int bc = this->columnCount();
   assert(count <= br);
   assert(at <= br);
   int bs = this->_children.size();
   int ar = br - count;
   //
   QVector<DovahKitLuaCompatibleItem*> rearranged;
   rearranged.resize(ar * bc);
   for (int i = 0; i < bs; ++i) {
      auto* item = this->_children[i];
      //
      int r = i / bc;
      int c = i % bc;
      if (r >= at) {
         r -= count;
         if (r < 0) {
            if (item)
               delete item;
            continue;
         }
      }
      rearranged[r * bc + c] = item;
   }
   this->_children = rearranged;
   this->_rows -= count;
}

std::pair<int, int> DovahKitLuaCompatibleItem::_coordinates() const noexcept {
   if (!this->_parent)
      return { -1, -1 };
   auto& list = this->_parent->_children;
   int   size = list.size();
   int   cc   = this->_parent->columnCount();
   for (int i = 0; i < size; ++i) {
      if (list[i] != this)
         continue;
      int r = i / cc;
      int c = i % cc;
      return { r, c };
   }
   assert(false && "Mismatched parent/child relationship!");
}

namespace {
   bool _sort_asc(const DovahKitLuaCompatibleItem* a, const DovahKitLuaCompatibleItem* b) {
      if (a) {
         if (b)
            return *a < *b;
         return true;
      }
      return false;
   }
   bool _sort_des(const DovahKitLuaCompatibleItem* a, const DovahKitLuaCompatibleItem* b) {
      if (b) {
         if (a)
            return *b < *a;
         return true;
      }
      return false;
   }
}
void DovahKitLuaCompatibleItem::_sortChildren(int column, Qt::SortOrder order, bool isTopLevel) {
   auto* model = this->_model;
   //
   QList<QPersistentModelIndex> parents;
   if (isTopLevel && model) {
      parents.push_back(this->index());
      model->layoutAboutToBeChanged(parents, QAbstractItemModel::VerticalSortHint);
   }
   auto& list = this->_children;
   int   size = list.size();
   QVector<DovahKitLuaCompatibleItem*> sorted = list;
   if (order == Qt::SortOrder::AscendingOrder) {
      qSort(sorted.begin(), sorted.end(), _sort_asc);
   } else {
      qSort(sorted.begin(), sorted.end(), _sort_des);
   }
   if (model) { // update persistent model indices
      auto persistent = model->persistentIndexList();
      if (!persistent.isEmpty()) {
         int cc = this->columnCount();
         //
         QModelIndexList change_from;
         QModelIndexList change_to;
         for (int i = 0; i < size; ++i) {
            int   r = i / cc;
            int   c = i % cc;
            auto* item = list[i];
            auto  from = model->createIndex(r, c, item);
            if (persistent.contains(from)) {
               auto to_i = sorted.indexOf(item);
               int  r = to_i / cc;
               int  c = to_i % cc;
               auto to = model->createIndex(r, c, item);
               change_from.push_back(from);
               change_to.push_back(to);
            }
         }
         if (!change_from.isEmpty()) {
            model->changePersistentIndexList(change_from, change_to);
         }
      }
   }
   std::swap(list, sorted);
   //
   for (auto* c : this->_children)
      if (c)
         c->_sortChildren(column, order, false);
   //
   if (isTopLevel && model) {
      model->layoutChanged(parents, QAbstractItemModel::VerticalSortHint);
   }
}

DovahKitLuaCompatibleItem::~DovahKitLuaCompatibleItem() {
   for (auto* c : this->_children)
      if (c)
         delete c;
   this->_children.clear();
}

bool DovahKitLuaCompatibleItem::operator<(const DovahKitLuaCompatibleItem& other) const {
   int role = Qt::DisplayRole;
   if (this->_model)
      role = this->_model->_sortRole;
   return this->data(role) < other.data(role);
}

void DovahKitLuaCompatibleItem::sortChildren(int column, Qt::SortOrder order) {
   this->_sortChildren(column, order, true);
}

void DovahKitLuaCompatibleItem::setColumnCount(int c) {
   assert(c >= 0);
   int bc   = this->columnCount();
   int diff = c - bc;
   if (!diff)
      return;
   else if (diff < 0)
      this->_deleteColumns(bc + diff, -diff);
   else
      this->_addColumns(bc - diff, diff);
}
void DovahKitLuaCompatibleItem::setRowCount(int r) {
   assert(r >= 0);
   int br   = this->rowCount();
   int diff = r - br;
   if (!diff)
      return;
   else if (diff < 0)
      this->_deleteRows(br + diff, -diff);
   else
      this->_addRows(br - diff, diff);
}

void DovahKitLuaCompatibleItem::appendColumn(const QList<DovahKitLuaCompatibleItem*>& items, bool clampRowCount) {
   this->insertColumn(this->columnCount(), items, clampRowCount);
}
void DovahKitLuaCompatibleItem::appendRow(const QList<DovahKitLuaCompatibleItem*>& items, bool clampColumnCount) {
   this->insertRow(this->rowCount(), items, clampColumnCount);
}
void DovahKitLuaCompatibleItem::appendRow(DovahKitLuaCompatibleItem* item) {
   this->appendRow({ item });
}

void DovahKitLuaCompatibleItem::insertColumn(int column, const QList<DovahKitLuaCompatibleItem*>& items, bool clampRowCount) {
   auto* model = this->_model;
   if (!model)
      return;
   int ar = items.size();
   if (!clampRowCount) {
      int br = this->rowCount();
      if (ar > br)
         this->insertRows(br + 1, ar - br);
      else
         ar = br;
   }
   model->beginInsertColumns(model->indexFromItem(this), column, column + 1);
   this->_addColumns(column, 1);
   for (int r = 0; r < ar; ++r) {
      int i = this->_childIndex(r, column);
      this->_children[i] = items[r];
   }
   model->endInsertColumns();
}
void DovahKitLuaCompatibleItem::insertColumns(int column, int count) {
   if (count <= 0)
      return;
   auto* model = this->_model;
   if (!model)
      return;
   model->beginInsertColumns(model->indexFromItem(this), column, column + count);
   this->_addColumns(column, count);
   model->endInsertColumns();
}
void DovahKitLuaCompatibleItem::insertRow(int row, const QList<DovahKitLuaCompatibleItem*>& items, bool clampColumnCount) {
   auto* model = this->_model;
   if (!model)
      return;
   int ac = items.size();
   if (!clampColumnCount) {
      int bc = this->columnCount();
      if (ac > bc)
         this->insertColumns(bc + 1, ac - bc);
      else
         ac = bc;
   }
   model->beginInsertRows(model->indexFromItem(this), row, row + 1);
   this->_addRows(row, 1);
   for (int c = 0; c < ac; ++c) {
      int i = this->_childIndex(row, c);
      this->_children[i] = items[c];
   }
   model->endInsertRows();
}
void DovahKitLuaCompatibleItem::insertRows(int row, int count) {
   if (count <= 0)
      return;
   auto* model = this->_model;
   if (!model)
      return;
   model->beginInsertRows(model->indexFromItem(this), row, row + count);
   this->_addRows(row, count);
   model->endInsertRows();
}

void DovahKitLuaCompatibleItem::removeColumn(int column) {
   this->removeColumns(column, 1);
}
void DovahKitLuaCompatibleItem::removeColumns(int column, int count) {
   int cc = this->columnCount();
   if (count <= 0 || column < 0 || column >= cc)
      return;
   if (count + column > cc) // clamp (count) to the end of the table
      count = cc - column;
   auto* model = this->_model;
   if (!model)
      return;
   model->beginRemoveColumns(model->indexFromItem(this), column, column + count - 1);
   this->_deleteColumns(column, count);
   model->endRemoveColumns();
}
void DovahKitLuaCompatibleItem::removeRow(int row) {
   this->removeRows(row, 1);
}
void DovahKitLuaCompatibleItem::removeRows(int row, int count) {
   int rc = this->rowCount();
   if (count <= 0 || row < 0 || row >= rc)
      return;
   if (count + row > rc) // clamp (count) to the end of the table
      count = rc - row;
   auto* model = this->_model;
   if (!model)
      return;
   model->beginRemoveRows(model->indexFromItem(this), row, row + count - 1);
   this->_deleteRows(row, count);
   model->endRemoveRows();
}

DovahKitLuaCompatibleItem* DovahKitLuaCompatibleItem::takeChild(int row, int column) noexcept {
   int i = this->_childIndex(row, column);
   if (i < 0)
      return nullptr;
   auto* child = this->_children[i];
   this->_children[i] = nullptr;
   return child;
}
QList<DovahKitLuaCompatibleItem*> DovahKitLuaCompatibleItem::takeColumn(int column) noexcept {
   QList<DovahKitLuaCompatibleItem*> out;
   int rc = this->rowCount();
   int cc = this->columnCount();
   if (column < 0 || column > cc)
      return out;
   out.reserve(rc);
   for (int i = 0; i < rc; ++i) {
      int p = this->_childIndex(i, column);
      if (p < 0)
         continue;
      out.push_back(this->_children[p]);
      this->_children[p] = nullptr;
   }
   this->_deleteColumns(column, 1);
   return out;
}
QList<DovahKitLuaCompatibleItem*> DovahKitLuaCompatibleItem::takeRow(int row) noexcept {
   QList<DovahKitLuaCompatibleItem*> out;
   int rc = this->rowCount();
   int cc = this->columnCount();
   if (row < 0 || row > rc)
      return out;
   out.reserve(cc);
   for (int i = 0; i < cc; ++i) {
      int p = this->_childIndex(row, i);
      if (p < 0)
         continue;
      out.push_back(this->_children[p]);
      this->_children[p] = nullptr;
   }
   this->_deleteRows(row, 1);
   return out;
}

/*virtual*/ QVariant DovahKitLuaCompatibleItem::data(int role) const {
   for (auto& e : this->_data)
      if (e.role == role)
         return e.value;
   if (role == Qt::ItemDataRole::ToolTipRole && this->_featureFlags.tooltipIsDisplay) {
      return this->data(Qt::ItemDataRole::DisplayRole);
   }
   return QVariant();
}
/*virtual*/ void DovahKitLuaCompatibleItem::setData(const QVariant& value, int role) {
   auto& list = this->_data;
   for (auto it = list.begin(); it != list.end(); ++it) {
      auto& e = *it;
      if (e.role == role) {
         if (!value.isValid()) {
            list.erase(it);
         } else {
            e.value = value;
         }
         this->emitDataChanged({ role });
         return;
      }
   }
   if (!value.isValid())
      return;
   _Data e;
   e.role  = role;
   e.value = value;
   list.push_back(e);
   this->emitDataChanged({ role });
}
void DovahKitLuaCompatibleItem::clearData() {
   QVector<int> roles;
   roles.reserve(this->_data.size());
   for (auto& e : this->_data)
      roles.push_back(e.role);
   this->_data.clear();
   this->emitDataChanged(roles);
}

QModelIndex DovahKitLuaCompatibleItem::index() const noexcept {
   if (!this->_model)
      return QModelIndex();
   return this->_model->indexFromItem(this);
}

DovahKitLuaCompatibleItem* DovahKitLuaCompatibleItem::child(int row, int column) const noexcept {
   int cc = this->columnCount();
   if (column > cc)
      return nullptr;
   int rc = this->rowCount();
   if (row > rc)
      return nullptr;
   int i = row * cc + column;
   assert(i < this->_children.size());
   return this->_children[i];
}

void DovahKitLuaCompatibleItem::emitDataChanged(const QVector<int>& roles) {
   if (!this->_model)
      return;
   this->_model->_onItemChanged(this, roles);
}
#pragma endregion

#pragma region DovahKitLuaCompatibleItemModel
void DovahKitLuaCompatibleItemModel::_onItemChanged(DovahKitLuaCompatibleItem* item, const QVector<int>& roles) {
   emit itemChanged(item);
   auto qmi = this->indexFromItem(item);
   emit dataChanged(qmi, qmi, roles);
}

DovahKitLuaCompatibleItemModel::DovahKitLuaCompatibleItemModel(QObject* parent) : QAbstractItemModel(parent) {
   this->root = new DovahKitLuaCompatibleItem;
}
DovahKitLuaCompatibleItemModel::~DovahKitLuaCompatibleItemModel() {
   if (this->root) {
      delete this->root;
      this->root = nullptr;
   }
}

void DovahKitLuaCompatibleItemModel::appendColumn(const QList<DovahKitLuaCompatibleItem*>& items) {
   this->insertColumn(this->root->columnCount(), items);
}
void DovahKitLuaCompatibleItemModel::appendRow(const QList<DovahKitLuaCompatibleItem*>& items) {
   this->insertRow(this->root->rowCount(), items);
}
void DovahKitLuaCompatibleItemModel::appendRow(DovahKitLuaCompatibleItem* item) {
   this->appendRow({ item });
}

void DovahKitLuaCompatibleItemModel::insertColumn(int column, const QList<DovahKitLuaCompatibleItem*>& items) {
   this->root->insertColumn(column, items);
}
bool DovahKitLuaCompatibleItemModel::insertColumn(int column, const QModelIndex& parent) {
   auto* item = parent.isValid() ? this->itemFromIndex(parent) : this->root;
   assert(item);
   item->insertColumns(column, 1);
}
void DovahKitLuaCompatibleItemModel::insertRow(int row, const QList<DovahKitLuaCompatibleItem*>& items) {
   this->root->insertRow(row, items);
}
void DovahKitLuaCompatibleItemModel::insertRow(int row, DovahKitLuaCompatibleItem* item) {
   this->insertRow(row, { item });
}
bool DovahKitLuaCompatibleItemModel::insertRow(int row, const QModelIndex& parent) {
   auto* item = parent.isValid() ? this->itemFromIndex(parent) : this->root;
   assert(item);
   item->insertRows(row, 1);
}

DovahKitLuaCompatibleItem* DovahKitLuaCompatibleItemModel::item(int row, int column) const noexcept {
   return this->root->child(row, column);
}
QModelIndex	DovahKitLuaCompatibleItemModel::indexFromItem(const DovahKitLuaCompatibleItem* item) const {
   if (item) {
      auto* parent = item->parent();
      if (parent) {
         auto c = item->_coordinates();
         return this->createIndex(c.first, c.second, parent);
      }
   }
   return QModelIndex();
}
DovahKitLuaCompatibleItem* DovahKitLuaCompatibleItemModel::itemFromIndex(const QModelIndex& index) const {
   if (!index.isValid())
      return nullptr;
   auto* parent = (DovahKitLuaCompatibleItem*) index.internalPointer();
   if (!parent)
      return nullptr;
   return parent->child(index.row(), index.column());
}

void DovahKitLuaCompatibleItemModel::beginInsertColumns(const QModelIndex& parent, int first, int last) {
   QAbstractItemModel::beginInsertColumns(parent, first, last);
   this->_changes.push_back({ parent, first, last });
}
void DovahKitLuaCompatibleItemModel::endInsertColumns() {
   QAbstractItemModel::endInsertColumns();
   auto change = this->_changes.pop();
   int  count  = change.last - change.first + 1;
   for (auto* observer : this->_observers) {
      auto& o = *observer;
      if (o.col < 0) // proceed only if this is a col or cell observer
         continue;
      if (o.col > change.first) // proceed only if the new cols were inserted before the observed col
         continue;
      if (o.parent != change.parent)
         continue;
      o.col += count;
   }
}
void DovahKitLuaCompatibleItemModel::beginInsertRows(const QModelIndex& parent, int first, int last) {
   QAbstractItemModel::beginInsertRows(parent, first, last);
   this->_changes.push_back({ parent, first, last });
}
void DovahKitLuaCompatibleItemModel::endInsertRows() {
   QAbstractItemModel::endInsertRows();
   auto change = this->_changes.pop();
   int  count  = change.last - change.first + 1;
   for (auto* observer : this->_observers) {
      auto& o = *observer;
      if (o.row < 0) // proceed only if this is a row or cell observer
         continue;
      if (o.row > change.first) // proceed only if the new rows were inserted before the observed row
         continue;
      if (o.parent != change.parent)
         continue;
      o.row += count;
   }
}
bool DovahKitLuaCompatibleItemModel::beginMoveColumns(const QModelIndex& sourceParent, int sourceFirst, int sourceLast, const QModelIndex& destinationParent, int destinationChild) {
   if (!QAbstractItemModel::beginMoveColumns(sourceParent, sourceFirst, sourceLast, destinationParent, destinationChild))
      return false;
   this->_changes.push_back({ sourceParent, sourceFirst, sourceLast, destinationParent, destinationChild });
   return true;
}
void DovahKitLuaCompatibleItemModel::endMoveColumns() {
   //
   // This function works basically the same way as endMoveRows.
   //
   auto change = this->_changes.pop();
   //
   bool fixup_source      = change.parent.parent() == change.destination.parent; // fixup bools for moves across parents
   bool fixup_destination = change.destination.parent.parent() == change.parent; // fixup bools for moves across parents
   QAbstractItemModel::endMoveColumns();
   //
   int F = change.first;
   int L = change.last;
   int D = change.destination.at;
   int items_moved = L - F + 1;
   if (change.parent == change.destination.parent) {
      //
      // Moving within the same parent.
      //
      // If I move the range [F, L] down to before D, inside of a list of N items, then:
      //
      //   - The range is moved to D - L - 1 + F.
      //   - The range is moved by D - L - 1.
      //   - The range [0, F) remains unchanged.
      //   - The range [F, L] moves down by D - L - 1.
      //   - The range (L, D) moves up by L - F + 1.
      //   - The range [D, N] remains unchanged.
      //
      // If I move the range [F, L] up to D, inside of a list of N items, then:
      //
      //   - The range [0, D) remains unchanged.
      //   - The range [D, F) moves down by L - F + 1.
      //   - The range [F, L] moves up by F - D.
      //   - The range (L, N] remains unchanged.
      //
      if (change.first < D) {
         //
         // Moved down.
         //
         for (auto* observer : this->_observers) {
            auto& o = *observer;
            if (o.col < 0) // not a col or cell observer
               continue;
            if (o.parent != change.parent) // in irrelevant parent
               continue;
            if (o.col >= F && o.col <= L) {
               o.col += D - L - 1;
            } else if (o.col > L && o.col < D) {
               o.col -= items_moved;
            }
         }
      } else {
         //
         // Moved up.
         //
         for (auto* observer : this->_observers) {
            auto& o = *observer;
            if (o.col < 0) // not a col or cell observer
               continue;
            if (o.parent != change.parent) // in irrelevant parent
               continue;
            if (o.col >= D && o.col < F) {
               o.col -= F - D;
            } else if (o.col >= F && o.col <= L) {
               o.col += items_moved;
            }
         }
      }
   } else {
      //
      // Moving from one parent to another.
      //
      // Moving items between parents can actually cause the parents themselves to change. Consider 
      // the following list:
      //
      //  - A
      //  - B
      //  - C
      //     - D
      //     - E
      //     - F
      //  - G
      //
      // If B is moved to above E, then C (which is change.destination.parent) will be displaced 
      // upward by one. The (fixup_destination) bool above tracks this.
      //
      // Conversely, if E is moved to above C, then C (which in this case is change.parent) will 
      // be displaced downward by one. The (fixup_source) bool above tracks this.
      //
      QModelIndex fixed_from = change.parent;
      QModelIndex fixed_to   = change.destination.parent;
      if (fixup_source) {
         fixed_from = this->createIndex(fixed_from.row(), fixed_from.column() + items_moved, fixed_from.internalPointer());
      }
      if (fixup_destination) {
         fixed_to = this->createIndex(fixed_to.row(), fixed_to.column() - items_moved, fixed_to.internalPointer());
      }
      for (auto* observer : this->_observers) {
         auto& o = *observer;
         if (o.col < 0) // not a row or cell observer
            continue;
         if (o.parent == change.parent) {
            //
            // Items were removed from this container, possibly including the observed row/cell.
            //
            o.parent = fixed_from;
            if (o.col >= change.first && o.col <= change.last) {
               o.parent = change.destination.parent;
               o.col -= change.first;
               o.col += change.destination.at;
            } else if (o.col > change.last) {
               o.col -= items_moved;
            }
         } else if (o.parent == change.destination.parent) {
            //
            // Items were inserted into this container, possibly including the observed row/cell.
            //
            o.parent = fixed_to;
            if (o.col >= change.destination.at) {
               o.col += items_moved;
            }
         }
      }
   }
}
bool DovahKitLuaCompatibleItemModel::beginMoveRows(const QModelIndex& sourceParent, int sourceFirst, int sourceLast, const QModelIndex& destinationParent, int destinationChild) {
   if (!QAbstractItemModel::beginMoveRows(sourceParent, sourceFirst, sourceLast, destinationParent, destinationChild))
      return false;
   this->_changes.push_back({ sourceParent, sourceFirst, sourceLast, destinationParent, destinationChild });
   return true;
}
void DovahKitLuaCompatibleItemModel::endMoveRows() {
   auto change = this->_changes.pop();
   //
   bool fixup_source      = change.parent.parent() == change.destination.parent; // fixup bools for moves across parents
   bool fixup_destination = change.destination.parent.parent() == change.parent; // fixup bools for moves across parents
   QAbstractItemModel::endMoveRows();
   //
   int F = change.first;
   int L = change.last;
   int D = change.destination.at;
   int items_moved = L - F + 1;
   if (change.parent == change.destination.parent) {
      //
      // Moving within the same parent.
      //
      // If I move the range [F, L] down to before D, inside of a list of N items, then:
      //
      //   - The range is moved to D - L - 1 + F.
      //   - The range is moved by D - L - 1.
      //   - The range [0, F) remains unchanged.
      //   - The range [F, L] moves down by D - L - 1.
      //   - The range (L, D) moves up by L - F + 1.
      //   - The range [D, N] remains unchanged.
      //
      // If I move the range [F, L] up to D, inside of a list of N items, then:
      //
      //   - The range [0, D) remains unchanged.
      //   - The range [D, F) moves down by L - F + 1.
      //   - The range [F, L] moves up by F - D.
      //   - The range (L, N] remains unchanged.
      //
      if (change.first < D) {
         //
         // Moved down.
         //
         for (auto* observer : this->_observers) {
            auto& o = *observer;
            if (o.row < 0) // not a row or cell observer
               continue;
            if (o.parent != change.parent) // in irrelevant parent
               continue;
            if (o.row >= F && o.row <= L) {
               o.row += D - L - 1;
            } else if (o.row > L && o.row < D) {
               o.row -= items_moved;
            }
         }
      } else {
         //
         // Moved up.
         //
         for (auto* observer : this->_observers) {
            auto& o = *observer;
            if (o.row < 0) // not a row or cell observer
               continue;
            if (o.parent != change.parent) // in irrelevant parent
               continue;
            if (o.row >= D && o.row < F) {
               o.row -= F - D;
            } else if (o.row >= F && o.row <= L) {
               o.row += items_moved;
            }
         }
      }
   } else {
      //
      // Moving from one parent to another.
      //
      // Moving items between parents can actually cause the parents themselves to change. Consider 
      // the following list:
      //
      //  - A
      //  - B
      //  - C
      //     - D
      //     - E
      //     - F
      //  - G
      //
      // If B is moved to above E, then C (which is change.destination.parent) will be displaced 
      // upward by one. The (fixup_destination) bool above tracks this.
      //
      // Conversely, if E is moved to above C, then C (which in this case is change.parent) will 
      // be displaced downward by one. The (fixup_source) bool above tracks this.
      //
      QModelIndex fixed_from = change.parent;
      QModelIndex fixed_to   = change.destination.parent;
      if (fixup_source) {
         fixed_from = this->createIndex(fixed_from.row() + items_moved, fixed_from.column(), fixed_from.internalPointer());
      }
      if (fixup_destination) {
         fixed_to = this->createIndex(fixed_to.row() - items_moved, fixed_to.column(), fixed_to.internalPointer());
      }
      for (auto* observer : this->_observers) {
         auto& o = *observer;
         if (o.row < 0) // not a row or cell observer
            continue;
         if (o.parent == change.parent) {
            //
            // Items were removed from this container, possibly including the observed row/cell.
            //
            o.parent = fixed_from;
            if (o.row >= change.first && o.row <= change.last) {
               o.parent = change.destination.parent;
               o.row -= change.first;
               o.row += change.destination.at;
            } else if (o.row > change.last) {
               o.row -= items_moved;
            }
         } else if (o.parent == change.destination.parent) {
            //
            // Items were inserted into this container, possibly including the observed row/cell.
            //
            o.parent = fixed_to;
            if (o.row >= change.destination.at) {
               o.row += items_moved;
            }
         }
      }
   }
}
void DovahKitLuaCompatibleItemModel::beginRemoveColumns(const QModelIndex& parent, int first, int last) {
   QAbstractItemModel::beginRemoveColumns(parent, first, last);
   this->_changes.push_back({ parent, first, last });
}
void DovahKitLuaCompatibleItemModel::endRemoveColumns() {
   QAbstractItemModel::endRemoveColumns();
   auto change = this->_changes.pop();
   int  count  = change.last - change.first + 1;
   for (auto* observer : this->_observers) {
      auto& o = *observer;
      if (o.col < 0) // proceed only if this is a col or cell observer
         continue;
      if (o.col >= change.first && o.col <= change.last) { // invalidate removed columns
         o.parent = QModelIndex();
         o.row   = -1;
         o.col   = -1;
         continue;
      }
      if (o.col > change.first) // proceed only if the new cols were removed from before the observed col
         continue;
      if (o.parent != change.parent)
         continue;
      o.col -= count;
   }
}
void DovahKitLuaCompatibleItemModel::beginRemoveRows(const QModelIndex& parent, int first, int last) {
   QAbstractItemModel::beginRemoveRows(parent, first, last);
   this->_changes.push_back({ parent, first, last });
}
void DovahKitLuaCompatibleItemModel::endRemoveRows() {
   QAbstractItemModel::endRemoveRows();
   auto change = this->_changes.pop();
   int  count  = change.last - change.first + 1;
   for (auto* observer : this->_observers) {
      auto& o = *observer;
      if (o.row < 0) // proceed only if this is a row or cell observer
         continue;
      if (o.row >= change.first && o.row <= change.last) { // invalidate removed rows
         o.parent = QModelIndex();
         o.row   = -1;
         o.col   = -1;
         continue;
      }
      if (o.row > change.first) // proceed only if the new rows were removed from before the observed row
         continue;
      if (o.parent != change.parent)
         continue;
      o.row -= count;
   }
}
void DovahKitLuaCompatibleItemModel::beginResetModel() {
   QAbstractItemModel::beginResetModel();
}
void DovahKitLuaCompatibleItemModel::endResetModel() {
   QAbstractItemModel::endResetModel();
   for (auto* observer : this->_observers) { // invalidate all observers
      observer->parent = QModelIndex();
      observer->row    = -1;
      observer->col    = -1;
   }
}
#pragma endregion