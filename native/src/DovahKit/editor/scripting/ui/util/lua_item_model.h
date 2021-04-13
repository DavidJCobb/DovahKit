#pragma once
#include <atomic>
#include <QAbstractItemModel>
#include <QStack>
#include <QStandardItemModel>

class DovahKitLuaCompatibleItemModel;

static_assert(false, "creating and destroying DovahKitLuaPersistentIndexRanges is not thread-safe, and in order to keep them in the script thread, we need it to be");
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
};

// Analogue to QStandardItem.
class DovahKitLuaCompatibleItem {
   friend class DovahKitLuaCompatibleItemModel;
   protected:
      struct _Data {
         int      role = -1;
         QVariant value;
      };

      QVector<_Data> _data;
      QVector<DovahKitLuaCompatibleItem*> _children; // 2D array represented as 1D
      DovahKitLuaCompatibleItem*      _parent = nullptr;
      DovahKitLuaCompatibleItemModel* _model  = nullptr;
      int _rows = 0;
      int _cols = 0;
      Qt::ItemFlags _itemFlags;
      struct {
         bool tooltipIsDisplay = false;
      } _featureFlags;

      inline int _childIndex(int row, int column) const noexcept {
         if (row < 0 || column < 0)
            return -1;
         if (row > this->rowCount())
            return -1;
         int cc = this->columnCount();
         if (column > cc)
            return -1;
         return column + (row * cc);
      }

      // These functions restructure our local table, but do not interact with the containing model.
      void _addColumns(int at, int count);
      void _addRows(int at, int count);
      void _deleteColumns(int at, int count);
      void _deleteRows(int at, int count);

      std::pair<int, int> _coordinates() const noexcept; // { row, col }

      void _sortChildren(int column, Qt::SortOrder order, bool isTopLevel);

   public:
      ~DovahKitLuaCompatibleItem();

      bool operator<(const DovahKitLuaCompatibleItem& other) const;

      void sortChildren(int column, Qt::SortOrder order = Qt::AscendingOrder);

      inline bool hasChildren() const noexcept { return this->rowCount() + this->columnCount() > 0; }

      inline int row() const noexcept { return this->_coordinates().first; }
      inline int column() const noexcept { return this->_coordinates().second; }

      inline int columnCount() const noexcept { return this->_cols; }
      inline int rowCount() const noexcept { return this->_rows; }
      void setColumnCount(int);
      void setRowCount(int);

      void appendColumn(const QList<DovahKitLuaCompatibleItem*>& items, bool clampRowCount = false);
      void appendRow(const QList<DovahKitLuaCompatibleItem*>& items, bool clampColumnCount = false);
      void appendRow(DovahKitLuaCompatibleItem* item);

      void insertColumn(int column, const QList<DovahKitLuaCompatibleItem*>& items, bool clampRowCount = false);
      void insertColumns(int column, int count);
      void insertRow(int row, const QList<DovahKitLuaCompatibleItem*>& items, bool clampColumnCount = false);
      void insertRows(int row, int count);

      void removeColumn(int column);
      void removeColumns(int column, int count);
      void removeRow(int row);
      void removeRows(int row, int count);

      DovahKitLuaCompatibleItem* takeChild(int row, int column = 0) noexcept;
      QList<DovahKitLuaCompatibleItem*> takeColumn(int column) noexcept;
      QList<DovahKitLuaCompatibleItem*> takeRow(int row) noexcept;

      virtual QVariant data(int role = Qt::UserRole + 1) const;
      virtual void setData(const QVariant& value, int role = Qt::UserRole + 1);
      void clearData();

      inline Qt::ItemFlags flags() const noexcept { return this->_itemFlags; }
      inline void setFlags(Qt::ItemFlags f) noexcept { this->_itemFlags = f; }

      QModelIndex index() const noexcept;

      DovahKitLuaCompatibleItem* child(int row, int column) const noexcept;
      inline DovahKitLuaCompatibleItem* parent() const noexcept { return this->_parent; }
      inline DovahKitLuaCompatibleItemModel* model() const noexcept { return this->_model; }

      inline bool tooltipsDefaultToDisplay() const noexcept { return this->_featureFlags.tooltipIsDisplay; };
      inline void setTooltipsDefaultToDisplay(bool b) noexcept { this->_featureFlags.tooltipIsDisplay = b; };

   protected:
      void emitDataChanged(const QVector<int>& roles);
};

// Analogue to QStandardItemModel.
class DovahKitLuaCompatibleItemModel : public QAbstractItemModel {
   Q_OBJECT;
   friend class DovahKitLuaCompatibleItem;
   protected:
      struct Change {
         QModelIndex parent;
         int first;
         int last;
         struct {
            QModelIndex parent;
            int at;
         } destination; // applicable for moves only
      };
      
      QVector<DovahKitLuaPersistentTableObserver*> _observers;
      QStack<Change> _changes;

      DovahKitLuaCompatibleItem* root = nullptr;
      struct {
         std::vector<DovahKitLuaCompatibleItem*> horizontal;
         std::vector<DovahKitLuaCompatibleItem*> vertical;
      } headerItems;
      int _sortRole = Qt::DisplayRole;

      void _onItemChanged(DovahKitLuaCompatibleItem*, const QVector<int>& roles);

   public:
      DovahKitLuaCompatibleItemModel(QObject* parent = nullptr);
      ~DovahKitLuaCompatibleItemModel();

      inline DovahKitLuaCompatibleItem* invisibleRootItem() const noexcept { return this->root; }

      void appendColumn(const QList<DovahKitLuaCompatibleItem*>& items);
      void appendRow(const QList<DovahKitLuaCompatibleItem*>& items);
      void appendRow(DovahKitLuaCompatibleItem* item);

      void insertColumn(int column, const QList<DovahKitLuaCompatibleItem*>& items);
      bool insertColumn(int column, const QModelIndex& parent = QModelIndex());
      void insertRow(int row, const QList<DovahKitLuaCompatibleItem*>& items);
      void insertRow(int row, DovahKitLuaCompatibleItem* item);
      bool insertRow(int row, const QModelIndex& parent = QModelIndex());

      DovahKitLuaCompatibleItem* item(int row, int column = 0) const noexcept;
      QModelIndex	indexFromItem(const DovahKitLuaCompatibleItem* item) const;
      DovahKitLuaCompatibleItem* itemFromIndex(const QModelIndex& index) const;

      static_assert(false, "we need data and setData overrides");
      static_assert(false, "we need headerData and setHeaderData overrides");

      inline int sortRole() const noexcept { return this->_sortRole; }
      inline void setSortRole(int r) noexcept { this->_sortRole = r; }

      void registerObserver(DovahKitLuaPersistentTableObserver*);
      void unregisterObserver(DovahKitLuaPersistentTableObserver*);

   protected:
      void beginInsertColumns(const QModelIndex& parent, int first, int last);
      void endInsertColumns();
      void beginInsertRows(const QModelIndex& parent, int first, int last);
      void endInsertRows();
      bool beginMoveColumns(const QModelIndex& sourceParent, int sourceFirst, int sourceLast, const QModelIndex& destinationParent, int destinationChild);
      void endMoveColumns();
      bool beginMoveRows(const QModelIndex& sourceParent, int sourceFirst, int sourceLast, const QModelIndex& destinationParent, int destinationChild);
      void endMoveRows();
      void beginRemoveColumns(const QModelIndex& parent, int first, int last);
      void endRemoveColumns();
      void beginRemoveRows(const QModelIndex& parent, int first, int last);
      void endRemoveRows();
      void beginResetModel();
      void endResetModel();

   signals:
      void itemChanged(DovahKitLuaCompatibleItem*);
};