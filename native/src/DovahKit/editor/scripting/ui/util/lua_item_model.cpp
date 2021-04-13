#include "lua_item_model.h"

#pragma region DovahKitLuaPersistentIndexRange
   #pragma region Data
      bool DovahKitLuaPersistentIndexRange::Data::operator==(const DovahKitLuaPersistentIndexRange::Data& other) const noexcept {
         return (this->range_start == other.range_start) && (this->range_end == other.range_end);
      }
      bool DovahKitLuaPersistentIndexRange::Data::operator!=(const DovahKitLuaPersistentIndexRange::Data& other) const noexcept {
         return (this->range_start != other.range_start) && (this->range_end != other.range_end);
      }

      /*static*/ DovahKitLuaPersistentIndexRange::Data* DovahKitLuaPersistentIndexRange::Data::_getOrCreate(const QModelIndex& start, const QModelIndex& end) {
         if (!start.isValid() || !end.isValid())
            return nullptr;
         auto* model = qobject_cast<DovahKitLuaCompatibleItemModel*>(start.model());
         if (!model)
            return nullptr;
         if (model != end.model())
            return nullptr;
         if (start.parent() != end.parent())
            return nullptr;
         //
         // Range seems valid. Let's commit it.
         //
         auto sit = model->_range_starts.constFind(start);
         auto eit = model->_range_ends.constFind(end);
         if (sit != model->_range_starts.cend())
            if (eit != model->_range_ends.cend())
               if (*sit == *eit)
                  return *sit; // this exact range already exists, so just share it
         auto* instance = new Data;
         instance->range_start = start;
         instance->range_end   = end;
         model->_range_starts.insert(start, instance);
         model->_range_ends.insert(end, instance);
         return instance;
      }
   #pragma endregion

   DovahKitLuaPersistentIndexRange::DovahKitLuaPersistentIndexRange() {
   }
   DovahKitLuaPersistentIndexRange::DovahKitLuaPersistentIndexRange(const QModelIndex& start, const QModelIndex& end) {
      if (this->_data = Data::_getOrCreate(start, end))
         ++this->_data->refcount;
   }
   DovahKitLuaPersistentIndexRange::DovahKitLuaPersistentIndexRange(DovahKitLuaPersistentIndexRange&& other) {
      *this = other;
   }

   DovahKitLuaPersistentIndexRange& DovahKitLuaPersistentIndexRange::operator=(DovahKitLuaPersistentIndexRange&& other) {
      if (auto* d = this->_data)
         if (--d->refcount == 0)
            delete d;
      this->_data = other._data;
      other._data = nullptr;
   }
   DovahKitLuaPersistentIndexRange& DovahKitLuaPersistentIndexRange::operator=(const DovahKitLuaPersistentIndexRange& other) {
      if (auto* d = this->_data)
         if (--d->refcount == 0)
            delete d;
      if (this->_data = other._data)
         ++this->_data->refcount;
   }

   bool DovahKitLuaPersistentIndexRange::operator==(const DovahKitLuaPersistentIndexRange& other) const noexcept {
      auto* a = this->_data;
      auto* b = other._data;
      if (a && b)
         if (*a == *b)
            return true;
      return a == b;
   }
   bool DovahKitLuaPersistentIndexRange::operator!=(const DovahKitLuaPersistentIndexRange& other) const noexcept {
      auto* a = this->_data;
      auto* b = other._data;
      if (a && b)
         if (*a != *b)
            return true;
      return a != b;
   }

   int DovahKitLuaPersistentIndexRange::columnAt(int i) const {
      if (!this->_data)
         return -1;
      const auto& s = this->_data->range_start;
      const auto& e = this->_data->range_end;
      i += s.column();
      if (i > e.column())
         return -1;
      return i;
   }
   int DovahKitLuaPersistentIndexRange::columnCount() const {
      if (!this->isValid())
         return 0;
      const auto& s = this->_data->range_start;
      const auto& e = this->_data->range_end;
      return e.column() - s.column() + 1;
   }
   int DovahKitLuaPersistentIndexRange::columnFirst() const {
      if (!this->isValid())
         return -1;
      return this->_data->range_start.column();
   }
   int DovahKitLuaPersistentIndexRange::columnLast() const {
      if (!this->isValid())
         return -1;
      return this->_data->range_end.column();
   }
   QModelIndex DovahKitLuaPersistentIndexRange::indexAt(int relativeRow, int relativeColumn) const {
      auto* model = this->model();
      if (!model)
         return QModelIndex();
      const auto& s = this->_data->range_start;
      const auto& e = this->_data->range_end;
      int row = relativeRow    + s.row();
      int col = relativeColumn + s.column();
      if (row > e.row() || col > e.column())
         return QModelIndex();
      return model->index(row, col, this->parent());
   }
   bool DovahKitLuaPersistentIndexRange::isValid() const {
      if (!this->_data)
         return false;
      auto& d = *this->_data;
      if (!d.range_start.isValid())
         return false;
      if (!d.range_end.isValid())
         return false;
      if (d.range_start.model() != d.range_end.model())
         return false;
      return true;
   }
   int DovahKitLuaPersistentIndexRange::rowAt(int i) const {
      if (!this->_data)
         return -1;
      const auto& s = this->_data->range_start;
      const auto& e = this->_data->range_end;
      i += s.row();
      if (i > e.row())
         return -1;
      return i;
   }
   int DovahKitLuaPersistentIndexRange::rowCount() const {
      if (!this->isValid())
         return 0;
      const auto& s = this->_data->range_start;
      const auto& e = this->_data->range_end;
      return e.row() - s.row() + 1;
   }
   int DovahKitLuaPersistentIndexRange::rowFirst() const {
      if (!this->isValid())
         return -1;
      return this->_data->range_start.row();
   }
   int DovahKitLuaPersistentIndexRange::rowLast() const {
      if (!this->isValid())
         return -1;
      return this->_data->range_end.row();
   }

   QVariant DovahKitLuaPersistentIndexRange::dataAt(int relativeRow, int relativeColumn, int role) const {
      auto qmi = this->indexAt(relativeRow, relativeColumn);
      if (qmi.isValid())
         return this->model()->data(qmi, role);
      return QVariant();
   }
   Qt::ItemFlags DovahKitLuaPersistentIndexRange::flagsAt(int relativeRow, int relativeColumn) const {
      auto qmi = this->indexAt(relativeRow, relativeColumn);
      if (qmi.isValid())
         return this->model()->flags(qmi);
      return Qt::ItemFlags();
   }
   const DovahKitLuaCompatibleItemModel* DovahKitLuaPersistentIndexRange::model() const {
      if (!this->isValid())
         return nullptr;
      return qobject_cast<DovahKitLuaCompatibleItemModel*>(this->_data->range_start.model());
   }
   QModelIndex DovahKitLuaPersistentIndexRange::parent() const {
      auto* model = this->model();
      if (!model)
         return QModelIndex();
      return model->parent(this->_data->range_start);
   }

   void DovahKitLuaPersistentIndexRange::swap(DovahKitLuaPersistentIndexRange& other) {
      std::swap(this->_data, other._data);
   }
#pragma endregion

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
#pragma endregion