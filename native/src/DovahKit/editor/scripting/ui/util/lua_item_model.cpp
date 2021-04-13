#include "lua_item_model.h"

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

void ObservableStandardItemModel::registerObserver(DovahKitLuaPersistentTableObserver* observer) {
   if (!observer)
      return;
   if (this->_observers.indexOf(observer) >= 0)
      return;
   this->_observers.push_back(observer);
}
void ObservableStandardItemModel::unregisterObserver(DovahKitLuaPersistentTableObserver* observer) {
   if (!observer)
      return;
   this->_observers.removeOne(observer);
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
      o.setAxis(orientation, p);
   }
}
void ObservableStandardItemModel::afterRemoval(Qt::Orientation orientation, const QModelIndex& parent, int first, int last) {
   int count = last - first + 1;
   for (auto* observer : this->_observers) {
      auto& o = *observer;
      int   p = o.axis(orientation);
      if (p < 0) // proceed only if this is a row or cell observer
         continue;
      if (p >= first && o.row <= last) { // invalidate removed rows
         o.invalidate();
         continue;
      }
      if (p > first) // proceed only if the new rows were removed from before the observed row
         continue;
      if (o.parent != parent)
         continue;
      o.setAxis(orientation, p);
   }
}
void ObservableStandardItemModel::beforeMove(Qt::Orientation, const QModelIndex& sourceParent, int sourceFirst, int sourceLast, const QModelIndex& destinationParent, int destinationAt) {
   Change change = { sourceParent, sourceFirst, sourceLast, destinationParent, destinationAt };
   change.fixup.source      = change.parent.parent() == change.destination.parent; // fixup bools for moves across parents
   change.fixup.destination = change.destination.parent.parent() == change.parent; // fixup bools for moves across parents
   this->_changes.push(change);
}
void ObservableStandardItemModel::afterMove(Qt::Orientation, const QModelIndex& fixedSourceParent, const QModelIndex& fixedDestinationParent) {
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
      for (auto* observer : this->_observers) {
         auto& o = *observer;
         if (o.row < 0) // not a row or cell observer
            continue;
         if (o.parent == change.parent) {
            //
            // Items were removed from this container, possibly including the observed row/cell.
            //
            o.parent = fixedSourceParent;
            if (o.row >= change.first && o.row <= change.last) {
               o.parent = fixedDestinationParent;
               o.row -= change.first;
               o.row += change.destination.at;
            } else if (o.row > change.last) {
               o.row -= items_moved;
            }
         } else if (o.parent == change.destination.parent) {
            //
            // Items were inserted into this container, possibly including the observed row/cell.
            //
            o.parent = fixedDestinationParent;
            if (o.row >= change.destination.at) {
               o.row += items_moved;
            }
         }
      }
   }
}
void ObservableStandardItemModel::afterReset() {
   for (auto* observer : this->_observers) // invalidate all observers
      observer->invalidate();
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
}
#pragma endregion