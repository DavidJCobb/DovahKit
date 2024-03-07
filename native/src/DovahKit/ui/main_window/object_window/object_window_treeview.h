#pragma once
#include <cstdint>
#include <optional>
#include <vector>
#include <QAbstractItemModel>
#include <QString>
#include <QTreeView>
#include "../../../dovah/core.h"
#include "../../generic/QLinedTreeView.h"

#include "./filter_info.h"

class ObjectWindowTreeModel;

class ObjectWindowTreeItem {
   friend class ObjectWindowTreeModel;
   protected:
      enum class type_t {
         root,
         top_level,
         form_type,
         filter,
      };

      ObjectWindowTreeItem() {}
      ~ObjectWindowTreeItem();

      static ObjectWindowTreeItem& make_top_level(const QString&);
      static ObjectWindowTreeItem& make_form_type(const QString&, dovah::form_type);
      static ObjectWindowTreeItem& make_filter(const QString&);

      QString name;
      QString full_filter;
      type_t  type      = type_t::form_type;
      std::optional<dovah::form_type> form_type;
      int     refcount  = 0;
      ObjectWindowTreeItem* parent = nullptr;
      QVector<ObjectWindowTreeItem*> children;

      ObjectWindowTreeItem& appendChild(ObjectWindowTreeItem&); // returns self, to allow chaining
      void takeChild(ObjectWindowTreeItem&);

      [[nodiscard]] inline ObjectWindowTreeItem* child(int i) const noexcept {
         if (i < 0 || i >= this->children.size())
            return nullptr;
         return this->children[i];
      }
      [[nodiscard]] inline int indexOf(ObjectWindowTreeItem* child) const noexcept { return this->children.indexOf(child); }
      [[nodiscard]] int indexOf(const QString& name) const noexcept;

      ObjectWindowTreeItem* findChildByFormType(dovah::form_type) const noexcept;

      void clear();
      dovah::form_type containingFormType() const noexcept; // for filters
      void gatherFormTypes(QVector<dovah::form_type>& out) const noexcept;

      //
      // There's only one circumstance where these should be used: you've called beginResetModel or 
      // something similar, you've added items en masse, and you want to sort them before calling 
      // endResetModel. In all other cases (e.g. sorting existing data, etc.), use the sort functions 
      // within the model itself.
      //
      void recursiveSort();
      void sort();
};

class ObjectWindowTreeModel : public QAbstractItemModel {
   Q_OBJECT
   public:
      using item_type = ObjectWindowTreeItem;
      //
   protected:
      struct {
         item_type* root   = nullptr;
         item_type* all    = nullptr;
         item_type* quests = nullptr;
      } _nodes;
      //
      static item_type* _itemFromIndex(const QModelIndex&) noexcept;
      item_type* _findFormTypeItem(dovah::form_type form_type) const noexcept;
      QModelIndex _indexOfItem(item_type*) const noexcept;
      QModelIndex _indexOfQuests() const noexcept;
      QModelIndex _indexOfAll() const noexcept;
      bool _removeRows(int row, int count, const QModelIndex& parent = QModelIndex());
      void _sortChildrenOf(item_type*);
      void _sortDescendantsOf(item_type*);
      //
      void _buildQuestFilters();
      void _buildAllModelPathFilters();
      void _clearFilters(item_type* root);
      void _removeFilter(item_type* root, const QString& full);
      void _addFilter(item_type* root, const QString& full, bool include_trailing = true);
      //
   public:
      ObjectWindowTreeModel(QObject* parent = nullptr);
      ~ObjectWindowTreeModel();
      
      #pragma region QAbstractItemModel overrides
         #pragma region Inlines
            inline item_type* invisibleRootItem() const noexcept { return this->_nodes.root; }
         #pragma endregion
         QModelIndex index(int row, int column, const QModelIndex& parent) const override;
         QModelIndex parent(const QModelIndex& index) const;
         int rowCount(const QModelIndex& parent) const override;
         int columnCount(const QModelIndex& item) const override;
         Qt::ItemFlags flags(const QModelIndex& index) const override;
         QVariant data(const QModelIndex& index, int role) const override;
         QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
      #pragma endregion

      QModelIndex indexOfAllCategory() const noexcept;
      QVector<dovah::form_type> formTypesFor(const QModelIndexList&) const noexcept;

      ui::object_window::filter_info getFilterInfoFor(const QModelIndexList&) const noexcept;
};

class ObjectWindowTree : public QLinedTreeView {
   Q_OBJECT
   public:
      ObjectWindowTree(QWidget* parent);
      using model_type = ObjectWindowTreeModel;
      //
      QVector<dovah::form_type> allPrimaryFormTypes() const noexcept;
      ui::object_window::filter_info filterInfo() const noexcept;
};