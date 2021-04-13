#pragma once
#include <atomic>
#include <QAbstractItemModel>
#include <QStandardItemModel>

class DovahKitLuaCompatibleItemModel;

class DovahKitLuaPersistentIndexRange {
   //
   // Similar in function and purpose to QPersistentModelIndex, but intended to represent a 
   // range of model indices.
   //
   // In theory, you could use a QPersistentModelIndex for the top-left item in a range, and 
   // a QPersistentModelIndex for the bottom-right item in a range, and this would allow you 
   // to track an arbitrary range of items (e.g. table rows, columns, or groups of cells) in 
   // a persistent manner. However, if either of those two specific items is invalidated 
   // (e.g. removed from the table), then the range becomes undefined; you no longer know 
   // where the range starts or ends because the start or end no longer exists.
   //
   // With DovahKitLuaPersistentIndexRange, if the start of the range is removed, then we 
   // push the persistent-start inward towards the end; and vice versa. For example, if an 
   // index range represents a table row and the table's first column is deleted, then the 
   // range will continue to refer to that particular row as long as there were more columns. 
   // Only if the range becomes empty will it become invalid.
   //
   // Of course, the model must actually maintain this object, which is why we've created the 
   // DovahKitLuaCompatibleItemModel class as a counterpart to QStandardItemModel.
   //
   friend class DovahKitLuaCompatibleItemModel;
   protected:
      struct Data {
         std::atomic<int> refcount = 0;
         QModelIndex range_start; // top-left
         QModelIndex range_end;   // bottom-right

         bool operator==(const Data&) const noexcept;
         bool operator!=(const Data&) const noexcept;

         static Data* _getOrCreate(const QModelIndex& start, const QModelIndex& end);
      };

      Data* _data = nullptr;
      
   public:
      DovahKitLuaPersistentIndexRange();
      DovahKitLuaPersistentIndexRange(const QModelIndex& start, const QModelIndex& end);
      DovahKitLuaPersistentIndexRange(DovahKitLuaPersistentIndexRange&&);

      DovahKitLuaPersistentIndexRange& operator=(DovahKitLuaPersistentIndexRange&&);
      DovahKitLuaPersistentIndexRange& operator=(const DovahKitLuaPersistentIndexRange&);

      bool operator==(const DovahKitLuaPersistentIndexRange&) const noexcept;
      bool operator!=(const DovahKitLuaPersistentIndexRange&) const noexcept;

      int columnAt(int) const; // returns the absolute column number of the column N entries into the range, or -1 if the range is empty or N is past the end of the range
      int columnCount() const;
      int columnFirst() const;
      int columnLast() const;
      QModelIndex indexAt(int relativeRow, int relativeColumn) const;
      bool isValid() const;
      int rowAt(int) const; // returns the absolute row number of the row N entries into the range, or -1 if the range is empty or N is past the end of the range
      int rowCount() const;
      int rowFirst() const;
      int rowLast() const;

      QVariant dataAt(int relativeRow, int relativeColumn, int role = Qt::DisplayRole) const;
      Qt::ItemFlags flagsAt(int relativeRow, int relativeColumn) const;
      const DovahKitLuaCompatibleItemModel* model() const;
      QModelIndex parent() const;

      void swap(DovahKitLuaPersistentIndexRange&);
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
   friend struct DovahKitLuaPersistentIndexRange::Data;
   friend class  DovahKitLuaCompatibleItem;
   protected:
      QMultiHash<QModelIndex, DovahKitLuaPersistentIndexRange::Data*> _range_starts;
      QMultiHash<QModelIndex, DovahKitLuaPersistentIndexRange::Data*> _range_ends;

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

      inline int sortRole() const noexcept { return this->_sortRole; }
      inline void setSortRole(int r) noexcept { this->_sortRole = r; }

   signals:
      void itemChanged(DovahKitLuaCompatibleItem*);
};