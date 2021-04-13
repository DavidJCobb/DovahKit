#pragma once
#include <atomic>
#include <QAbstractItemModel>
#include <QStack>
#include <QStandardItemModel>

static_assert(false, "creating and destroying DovahKitLuaPersistentTableObserver is not thread-safe, and in order to keep them in the script thread, we need it to be");
//
// The script VM already has a main thread loop. Could we have the index-ranges be stored 
// in the script VM, with the Lua wrappers referring to them obliquely (e.g. a vector of 
// index-ranges, and the wrappers refer to positions in the vector)?
//
// We could potentially have a singleton take responsibility for creating and destroying 
// these things, always on the main thread, and just try to rig it up so that all threads 
// ask that singleton to perform those tasks.
//

struct DovahKitLuaPersistentTableObserver {
   QModelIndex parent;
   int row = -1;
   int col = -1;

   inline const int axis(Qt::Orientation o) const noexcept {
      if (o == Qt::Orientation::Horizontal)
         return this->col;
      return this->row;
   }
   inline void setAxis(Qt::Orientation o, int i) noexcept {
      if (o == Qt::Orientation::Horizontal)
         this->col = i;
      else
         this->row = i;
   }

   inline void invalidate() noexcept {
      this->parent = QModelIndex();
      this->row = this->col = -1;
   }
   inline bool isValid() const noexcept {
      if (this->row < 0)
         if (this->col < 0)
            return false;
      return this->parent.isValid();
   }
};

class ObservableStandardItemModel : public QStandardItemModel {
   Q_OBJECT;
   protected:
      struct Change {
         QModelIndex parent;
         int first;
         int last;
         struct {
            QModelIndex parent;
            int at;
         } destination; // applicable for moves only
         struct {
            bool source      = false;
            bool destination = false;
         } fixup; // applicable for moves only
      };

      // Struct for coping with layoutChanged.
      struct ObserverCacheEntry {
         QStandardItem* parent = nullptr;
         QStandardItem* row    = nullptr; // leftmost entry in row
         QStandardItem* col    = nullptr; // topmost entry in col
         //
         #if _DEBUG
         struct {
            int parent_row_count = -1;
            int parent_col_count = -1;
         } debug;
         #endif
      };
      
      QVector<DovahKitLuaPersistentTableObserver*> _observers;
      QVector<ObserverCacheEntry> _observer_cache;
      QStack<Change> _changes;

   public:
      ObservableStandardItemModel(QObject* parent = nullptr);

      void registerObserver(DovahKitLuaPersistentTableObserver*);
      void unregisterObserver(DovahKitLuaPersistentTableObserver*);

   protected:
      void afterInsertion(Qt::Orientation, const QModelIndex& parent, int first, int last);
      void afterRemoval(Qt::Orientation, const QModelIndex& parent, int first, int last);
      void beforeMove(Qt::Orientation, const QModelIndex& sourceParent, int sourceFirst, int sourceLast, const QModelIndex& destinationParent, int destinationAt);
      void afterMove(Qt::Orientation, const QModelIndex& fixedSourceParent, const QModelIndex& fixedDestinationParent);
      void afterReset();

      // QAbstractItemModel::layoutChanged is used by:
      //
      //  - QStandardItem::setChild
      //     - QStandardItemModel::dropMimeData, both directly and in a private helper function called there
      //     - QStandardItemModel::itemFromIndex, if the index is valid but no item is there
      //     - QStandardItemModel::setItem
      //  - QStandardItem::sortChildren
      //
      // Neither of the above functions delete a child. If you orphan a child with setChild(r, c, nullptr), 
      // the orphaned child simply remains in memory (and leaks, if you weren't tracking it elsewhere).
      //
      // We can handle layoutChanged signals if and only if they do not involve the deletion of children 
      // from the model.
      //
      void beforeLayoutChange();
      void afterLayoutChange();
};