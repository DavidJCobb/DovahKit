#pragma once
#include <QStack>
#include <QStandardItemModel>

class ObservableStandardItemModel;

struct ObservableStandardItemModelObserver {
   ObservableStandardItemModel* model = nullptr;
   QModelIndex parent; // allowed to be invalid, as that's needed to refer to top-level items in the model
   int row = -1;
   int col = -1;

   ~ObservableStandardItemModelObserver();

   static constexpr auto rowOrientation = Qt::Orientation::Vertical;
   static constexpr auto colOrientation = Qt::Orientation::Horizontal;

   inline const int axis(Qt::Orientation o) const noexcept {
      if (o == colOrientation)
         return this->col;
      return this->row;
   }
   inline void setAxis(Qt::Orientation o, int i) noexcept {
      if (o == colOrientation)
         this->col = i;
      else
         this->row = i;
   }

   inline void invalidate() noexcept {
      this->model  = nullptr;
      this->parent = QModelIndex();
      this->row    = this->col = -1;
   }
   inline bool isValid() const noexcept {
      if (!this->model)
         return false;
      if (this->row < 0)
         if (this->col < 0)
            return false;
      return true;
   }

   inline bool isRow() const noexcept {
      return this->row >= 0 && this->col < 0;
   }
   inline bool isColumn() const noexcept {
      return this->col >= 0 && this->row < 0;
   }
   inline bool isCell() const noexcept {
      return this->col >= 0 && this->row >= 0;
   }

   QStandardItem* item(int offset = 0) const noexcept;
   QModelIndex itemIndex(int offset = 0) const noexcept;

   void unregister();
};

class ObservableStandardItemModel : public QStandardItemModel {
   Q_OBJECT;
   protected:
      using  RoleDefault = std::pair<int, QVariant>;
      struct RoleDefaultSet {
         QVector<RoleDefault> rows;
         QVector<RoleDefault> cols;

         inline QVariant forRow(int r) const noexcept {
            for (auto& d : this->rows)
               if (d.first == r)
                  return d.second;
            return QVariant();
         }
         inline QVariant forCol(int r) const noexcept {
            for (auto& d : this->cols)
               if (d.first == r)
                  return d.second;
            return QVariant();
         }
         QVector<RoleDefault>& setByAxis(Qt::Orientation o) noexcept {
            return (o == rowOrientation) ? rows : cols;
         }
         const QVector<RoleDefault>& setByAxis(Qt::Orientation o) const noexcept {
            return (o == rowOrientation) ? rows : cols;
         }
      };

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
      
      QVector<ObservableStandardItemModelObserver*> _observers;
      QVector<ObserverCacheEntry> _observer_cache;
      QStack<Change> _changes;
      QMap<Qt::ItemDataRole, RoleDefaultSet> _defaultsByRole;

   public:
      ObservableStandardItemModel(QObject* parent = nullptr);
      ~ObservableStandardItemModel();

      static constexpr auto rowOrientation = ObservableStandardItemModelObserver::rowOrientation;
      static constexpr auto colOrientation = ObservableStandardItemModelObserver::colOrientation;

      ObservableStandardItemModelObserver* getOrCreateRegisteredObserver(const QModelIndex& cell);
      ObservableStandardItemModelObserver* getOrCreateRegisteredObserver(const QModelIndex& parent, Qt::Orientation, int offset);
      void registerObserver(ObservableStandardItemModelObserver*);
      void unregisterObserver(ObservableStandardItemModelObserver*);

      // top-level data only
      QVariant getDefaultDataForSpan(int role, Qt::Orientation, int pos) const noexcept;
      void setDefaultDataForSpan(int role, Qt::Orientation, int pos, QVariant data); // an invalid QVariant will clear any existing default for the span

      virtual QVariant data(const QModelIndex& index, int role) const override;

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