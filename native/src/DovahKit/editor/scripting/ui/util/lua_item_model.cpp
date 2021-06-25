#include "lua_item_model.h"

#pragma region ObservableStandardItemModelObserver
ObservableStandardItemModelObserver::~ObservableStandardItemModelObserver() {
   this->unregister();
}

QStandardItem* ObservableStandardItemModelObserver::item(int offset) const noexcept {
   if (!this->model)
      return nullptr;
   int r = this->row;
   int c = this->col;
   if (r < 0) {
      if (c < 0)
         return nullptr;
      r = offset;
   } else if (c < 0)
      c = offset;
   auto* parent = this->model->invisibleRootItem();
   if (this->parent.isValid())
      parent = this->model->itemFromIndex(this->parent);
   return parent->child(r, c);
}
QModelIndex ObservableStandardItemModelObserver::itemIndex(int offset) const noexcept {
   auto* item = this->item(offset);
   if (item)
      return this->model->indexFromItem(item);
   return QModelIndex();
}

void ObservableStandardItemModelObserver::unregister() {
   auto* model = this->model;
   this->invalidate();
   if (!model)
      return;
   model->unregisterObserver(this);
}
#pragma endregion

#pragma region ObservableStandardItemModel
ObservableStandardItemModel::ObservableStandardItemModel(QObject* parent) : QStandardItemModel(parent) {
   #pragma region rows
      QObject::connect(this, &QStandardItemModel::rowsInserted, this, [this](const QModelIndex& parent, int first, int last) {
         this->afterInsertion(Qt::Orientation::Vertical, parent, first, last);
      });
      QObject::connect(this, &QStandardItemModel::rowsRemoved, this, [this](const QModelIndex& parent, int first, int last) {
         this->afterRemoval(Qt::Orientation::Vertical, parent, first, last);
      });
      QObject::connect(this, &QStandardItemModel::rowsAboutToBeMoved, this, [this](const QModelIndex& sourceParent, int sourceFirst, int sourceLast, const QModelIndex& destinationParent, int destinationAt) {
         this->beforeMove(Qt::Orientation::Vertical, sourceParent, sourceFirst, sourceLast, destinationParent, destinationAt);
      });
      QObject::connect(this, &QStandardItemModel::rowsMoved, this, [this](const QModelIndex& sourceParent, int sourceFirst, int sourceLast, const QModelIndex& destinationParent, int destinationAt) {
         this->afterMove(Qt::Orientation::Vertical, sourceParent, destinationParent);
      });
   #pragma endregion
   #pragma region cols
      QObject::connect(this, &QStandardItemModel::columnsInserted, this, [this](const QModelIndex& parent, int first, int last) {
         this->afterInsertion(Qt::Orientation::Horizontal, parent, first, last);
      });
      QObject::connect(this, &QStandardItemModel::columnsRemoved, this, [this](const QModelIndex& parent, int first, int last) {
         this->afterRemoval(Qt::Orientation::Horizontal, parent, first, last);
      });
      QObject::connect(this, &QStandardItemModel::columnsAboutToBeMoved, this, [this](const QModelIndex& sourceParent, int sourceFirst, int sourceLast, const QModelIndex& destinationParent, int destinationAt) {
         this->beforeMove(Qt::Orientation::Horizontal, sourceParent, sourceFirst, sourceLast, destinationParent, destinationAt);
      });
      QObject::connect(this, &QStandardItemModel::columnsMoved, this, [this](const QModelIndex& sourceParent, int sourceFirst, int sourceLast, const QModelIndex& destinationParent, int destinationAt) {
         this->afterMove(Qt::Orientation::Horizontal, sourceParent, destinationParent);
      });
   #pragma endregion
   QObject::connect(this, &QStandardItemModel::modelReset, this, &ObservableStandardItemModel::afterReset);
   QObject::connect(this, &QStandardItemModel::layoutAboutToBeChanged, this, &ObservableStandardItemModel::beforeLayoutChange);
   QObject::connect(this, &QStandardItemModel::layoutChanged, this, &ObservableStandardItemModel::afterLayoutChange);
}
ObservableStandardItemModel::~ObservableStandardItemModel() {
   for (auto* o : this->_observers)
      o->invalidate();
}

ObservableStandardItemModelObserver* ObservableStandardItemModel::getOrCreateRegisteredObserver(const QModelIndex& cell) {
   if (!cell.isValid())
      return nullptr;
   auto* item = this->itemFromIndex(cell);
   for (auto* o : this->_observers)
      if (o->isCell())
         if (o->item() == item)
            return o;
   //
   // There is no existing and registered observer for this cell. Create one.
   //
   auto* o = new ObservableStandardItemModelObserver;
   o->model  = this;
   o->parent = cell.parent();
   o->row    = item->row();
   o->col    = item->column();
   this->_observers.push_back(o);
   return o;
}
ObservableStandardItemModelObserver* ObservableStandardItemModel::getOrCreateRegisteredObserver(const QModelIndex& parent, Qt::Orientation main, int offset) {
   if (offset < 0)
      return nullptr;
   {
      auto* parentItem = parent.isValid() ? this->itemFromIndex(parent) : this->invisibleRootItem();
      if (!parentItem)
         return nullptr;
      int bound;
      if (main == rowOrientation)
         bound = parentItem->rowCount();
      else
         bound = parentItem->columnCount();
      if (bound <= offset)
         return nullptr;
   }
   auto cross = (main == rowOrientation) ? colOrientation : rowOrientation;
   for (auto* o : this->_observers) {
      int m = o->axis(main);
      int c = o->axis(cross);
      if (c >= 0) // cell or cross-axis
         continue;
      if (m != offset)
         continue;
      bool valid = o->parent.isValid();
      if (valid != parent.isValid())
         continue;
      if (valid && o->parent != parent)
         continue;
      return o;
   }
   //
   // There is no existing and registered observer for this cell. Create one, if the 
   // desired offset is in-bounds.
   //
   auto* o = new ObservableStandardItemModelObserver;
   o->model  = this;
   o->parent = parent;
   o->setAxis(main, offset);
   this->_observers.push_back(o);
   return o;

}
void ObservableStandardItemModel::registerObserver(ObservableStandardItemModelObserver* observer) {
   if (!observer)
      return;
   if (this->_observers.indexOf(observer) >= 0)
      return;
   this->_observers.push_back(observer);
}
void ObservableStandardItemModel::unregisterObserver(ObservableStandardItemModelObserver* observer) {
   if (!observer)
      return;
   this->_observers.removeOne(observer);
}

QVariant ObservableStandardItemModel::getDefaultDataForSpan(int role, Qt::Orientation orientation, int pos) const noexcept {
   auto it = this->_defaultsByRole.find((Qt::ItemDataRole)role);
   if (it == this->_defaultsByRole.end())
      return QVariant();
   auto& list = it->setByAxis(orientation);
   for (auto& pair : list)
      if (pair.first == pos)
         return pair.second;
   return QVariant();
}
void ObservableStandardItemModel::setDefaultDataForSpan(int role, Qt::Orientation orientation, int pos, QVariant data) {
   QModelIndex qmi_tl;
   QModelIndex qmi_br;
   bool empty = true;
   if (orientation == rowOrientation) {
      int cross_count = this->columnCount();
      qmi_tl = this->index(pos, 0);
      qmi_br = this->index(pos, cross_count - 1);
      if (cross_count > 0)
         empty = false;
   } else {
      int cross_count = this->rowCount();
      qmi_tl = this->index(0, pos);
      qmi_br = this->index(cross_count - 1, pos);
      if (cross_count > 0)
         empty = false;
   }
   //
   auto it = this->_defaultsByRole.find((Qt::ItemDataRole)role);
   if (it == this->_defaultsByRole.end()) {
      if (!data.isValid())
         return;
      it = this->_defaultsByRole.insert((Qt::ItemDataRole)role, RoleDefaultSet());
   }
   auto& list = it->setByAxis(orientation);
   for (auto jt = list.begin(); jt != list.end(); ++jt) {
      auto& pair = *jt;
      if (pair.first == pos) {
         if (data.isValid()) {
            pair.second = data;
         } else {
            list.erase(jt);
         }
         if (!empty)
            emit dataChanged(qmi_tl, qmi_br, { role });
         return;
      }
   }
   list.append({ pos, data });
   if (!empty)
      emit dataChanged(qmi_tl, qmi_br, { role });
}

QVariant ObservableStandardItemModel::data(const QModelIndex& index, int role) const {
   if (role == Qt::FontRole) {
      //
      // Special-case behavior for FontRole: if a cell specifies some font properties, 
      // its containing row specifies some others, and its containing column specifies 
      // yet others, then these three sets of properties should be coalesced, with the 
      // cell properties prioritized over the row properties and those over the column 
      // properties.
      //
      if (!index.isValid())
         return QVariant();
      QVariant cell = QStandardItemModel::data(index, role);
      QVariant row;
      QVariant col;
      //
      auto it = this->_defaultsByRole.find((Qt::ItemDataRole)role);
      if (it != this->_defaultsByRole.end()) {
         auto& entry = *it;
         row = entry.forRow(index.row());
         col = entry.forCol(index.column());
      }
      //
      if (!cell.isValid() || cell.type() == QMetaType::QFont) {
         QFont resolved = cell.value<QFont>();
         resolved = resolved.resolve(row.value<QFont>());
         resolved = resolved.resolve(col.value<QFont>());
         return resolved;
      }
      if (cell.isValid())
         return cell;
      if (row.isValid())
         return row;
      if (col.isValid())
         return col;
      return QVariant();
   }
   //
   auto value = QStandardItemModel::data(index, role);
   if (!value.isValid()) {
      if (index.isValid() && !index.parent().isValid()) { // top-level item
         auto it = this->_defaultsByRole.find((Qt::ItemDataRole)role);
         if (it != this->_defaultsByRole.end()) {
            auto& entry = *it;
            int   row   = index.row();
            int   col   = index.column();
            value = entry.forRow(row);
            if (!value.isValid())
               value = entry.forCol(col);
         }
      }
   }
   return value;
}

void ObservableStandardItemModel::afterInsertion(Qt::Orientation orientation, const QModelIndex& parent, int first, int last) {
   int count = last - first + 1;
   for (auto* observer : this->_observers) {
      auto& o = *observer;
      int   p = o.axis(orientation);
      if (p < 0) // proceed only if this is a col or cell observer
         continue;
      if (p > first) // proceed only if the new cols were inserted before the observed col
         continue;
      if (o.parent != parent)
         continue;
      o.setAxis(orientation, p + count);
   }
   for (auto& dataset : this->_defaultsByRole) {
      auto& list = dataset.setByAxis(orientation);
      for (auto& pair : list) {
         int p = pair.first;
         if (p > first)
            continue;
         pair.first = p + count;
      }
   }
}
void ObservableStandardItemModel::afterRemoval(Qt::Orientation orientation, const QModelIndex& parent, int first, int last) {
   int  count = last - first + 1;
   bool any_invalidated = false;
   for (auto* observer : this->_observers) {
      auto& o = *observer;
      int   p = o.axis(orientation);
      if (p < 0) // proceed only if this is a row or cell observer
         continue;
      if (p >= first && o.row <= last) { // invalidate removed rows
         o.invalidate();
         any_invalidated = true;
         continue;
      }
      if (p > first) // proceed only if the new rows were removed from before the observed row
         continue;
      if (o.parent != parent)
         continue;
      o.setAxis(orientation, p - count);
   }
   if (any_invalidated) {
      //
      // If we invalidated any observers, we now need to remove them from our list. Invalidating an observer 
      // severs its model pointer, which means that if it's deleted later, it won't be able to let us know. 
      // We have to get rid of it now.
      //
      any_invalidated = false;
      //
      auto& list = this->_observers;
      int   size = list.size();
      for (int i = 0; i < size; ++i) {
         auto* o = list[i];
         if (o->model == this)
            continue;
         list[i] = nullptr;
         any_invalidated = true;
      }
      if (any_invalidated)
         list.removeAll(nullptr);
   }
   for (auto& dataset : this->_defaultsByRole) {
      auto& list = dataset.setByAxis(orientation);
      for (auto& pair : list) {
         int p = pair.first;
         if (p > first)
            continue;
         pair.first = p - count;
      }
   }
}
void ObservableStandardItemModel::beforeMove(Qt::Orientation, const QModelIndex& sourceParent, int sourceFirst, int sourceLast, const QModelIndex& destinationParent, int destinationAt) {
   Change change = { sourceParent, sourceFirst, sourceLast, destinationParent, destinationAt };
   change.fixup.source      = change.parent.parent() == change.destination.parent; // fixup bools for moves across parents
   change.fixup.destination = change.destination.parent.parent() == change.parent; // fixup bools for moves across parents
   this->_changes.push(change);
}
void ObservableStandardItemModel::afterMove(Qt::Orientation orientation, const QModelIndex& fixedSourceParent, const QModelIndex& fixedDestinationParent) {
   auto change = this->_changes.pop();
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
            int   p = o.axis(orientation);
            if (p < 0) // not a row or cell observer
               continue;
            if (o.parent != change.parent) // in irrelevant parent
               continue;
            if (p >= F && p <= L) {
               p += D - L - 1;
            } else if (p > L && p < D) {
               p -= items_moved;
            }
            o.setAxis(orientation, p);
         }
         if (!change.parent.isValid()) { // top-level elements
            for (auto& dataset : this->_defaultsByRole) {
               auto& list = dataset.setByAxis(orientation);
               for (auto& pair : list) {
                  int p = pair.first;
                  if (p >= F && p <= L) {
                     p += D - L - 1;
                  } else if (p > L && p < D) {
                     p -= items_moved;
                  }
                  pair.first = p;
               }
            }
         }
      } else {
         //
         // Moved up.
         //
         for (auto* observer : this->_observers) {
            auto& o = *observer;
            int   p = o.axis(orientation);
            if (p < 0) // not a row or cell observer
               continue;
            if (o.parent != change.parent) // in irrelevant parent
               continue;
            if (p >= D && p < F) {
               p -= F - D;
            } else if (p >= F && p <= L) {
               p += items_moved;
            }
            o.setAxis(orientation, p);
         }
         if (!change.parent.isValid()) { // top-level elements
            for (auto& dataset : this->_defaultsByRole) {
               auto& list = dataset.setByAxis(orientation);
               for (auto& pair : list) {
                  int p = pair.first;
                  if (p >= D && p < F) {
                     p -= F - D;
                  } else if (p >= F && p <= L) {
                     p += items_moved;
                  }
                  pair.first = p;
               }
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
      for (auto* observer : this->_observers) {
         auto& o = *observer;
         int   p = o.axis(orientation);
         if (o.row < 0) // not a row or cell observer
            continue;
         if (o.parent == change.parent) {
            //
            // Items were removed from this container, possibly including the observed row/cell.
            //
            o.parent = fixedSourceParent;
            if (p >= change.first && p <= change.last) {
               o.parent = fixedDestinationParent;
               p -= change.first;
               p += change.destination.at;
            } else if (p > change.last) {
               p -= items_moved;
            }
         } else if (o.parent == change.destination.parent) {
            //
            // Items were inserted into this container, possibly including the observed row/cell.
            //
            o.parent = fixedDestinationParent;
            if (p >= change.destination.at) {
               p += items_moved;
            }
         }
         o.setAxis(orientation, p);
      }
      if (!change.parent.isValid()) { // top-level element removal
         for (auto& dataset : this->_defaultsByRole) {
            auto& list = dataset.setByAxis(orientation);
            for (auto& pair : list)
               if (pair.first > change.last)
                  pair.first -= items_moved;
         }
      }
      if (!change.destination.parent.isValid()) { // top-level element addition
         for (auto& dataset : this->_defaultsByRole) {
            auto& list = dataset.setByAxis(orientation);
            for (auto& pair : list)
               if (pair.first >= change.destination.at)
                  pair.first += items_moved;
         }
      }
   }
}
void ObservableStandardItemModel::afterReset() {
   for (auto* observer : this->_observers) { // invalidate all observers
      observer->invalidate();
   }
   this->_observers.clear();
   this->_defaultsByRole.clear();
}
void ObservableStandardItemModel::beforeLayoutChange() {
   auto& cache = this->_observer_cache;
   auto& list  = this->_observers;
   int   size  = list.size();
   cache.resize(size);
   for (int i = 0; i < size; ++i) {
      auto& o = *list[i];
      auto& c = cache[i];
      c.parent = this->itemFromIndex(o.parent);
      c.row    = c.parent->child(o.row, 0);
      c.col    = c.parent->child(0, o.col);
      #if _DEBUG
         c.debug.parent_row_count = c.parent->rowCount();
         c.debug.parent_col_count = c.parent->columnCount();
      #endif
   }
}
void ObservableStandardItemModel::afterLayoutChange() {
   auto& cache = this->_observer_cache;
   auto& list  = this->_observers;
   int   size  = list.size();
   for (int i = 0; i < size; ++i) {
      auto& o = *list[i];
      auto& c = cache[i];
      #if _DEBUG
         assert(c.parent->rowCount()    == c.debug.parent_row_count && "ObservableStandardItemModel cannot cope with layoutChanged events that delete children!");
         assert(c.parent->columnCount() == c.debug.parent_col_count && "ObservableStandardItemModel cannot cope with layoutChanged events that delete children!");
      #endif
      o.parent = this->indexFromItem(c.parent);
      o.row    = c.row->row();
      o.col    = c.col->column();
   }
   cache.clear();
   //
   this->_defaultsByRole.clear(); // TODO: figure out how to get this working
}
#pragma endregion