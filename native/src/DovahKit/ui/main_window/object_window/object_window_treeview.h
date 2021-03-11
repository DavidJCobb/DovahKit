#pragma once
#include <cstdint>
#include <vector>
#include <QAbstractItemModel>
#include <QString>
#include <QTreeView>
#include "../../../dovah/core.h"
#include "../../generic/QLinedTreeView.h"

class ObjectWindowTreeModel;

class ObjectWindowTreeItem {
   friend class ObjectWindowTreeModel;
   protected:
      static constexpr int no_form_type_filter = -1;
      enum class item_type {
         root,
         top_level,
         form_type,
         filter,
      };

      ObjectWindowTreeItem() {}

      static ObjectWindowTreeItem& make_top_level(const QString&);
      static ObjectWindowTreeItem& make_form_type(const QString&, int);
      static ObjectWindowTreeItem& make_filter(const QString&);

      QString   name;
      item_type type      = item_type::form_type;
      int       form_type = no_form_type_filter;
      int       refcount  = 0;
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

      void gatherFormTypes(QVector<dovah::form_type_t>& out) const noexcept;
};

class ObjectWindowTreeModel : public QAbstractItemModel {
   Q_OBJECT
   public:
      using item_type = ObjectWindowTreeItem;
   protected:
      item_type* root   = nullptr;
      item_type* all    = nullptr;
      item_type* quests = nullptr;
      //
      static item_type* _itemFromIndex(const QModelIndex&) noexcept;
      QModelIndex _indexOfItem(item_type*) const noexcept;
      QModelIndex _indexOfQuests() const noexcept;
      QModelIndex _indexOfAll() const noexcept;
      bool _removeRows(int row, int count, const QModelIndex& parent = QModelIndex());
      //
   public:
      ObjectWindowTreeModel(QObject* parent = nullptr);
      ~ObjectWindowTreeModel();
      
      #pragma region QAbstractItemModel overrides
         #pragma region Inlines
            inline item_type* invisibleRootItem() const noexcept { return this->root; }
         #pragma endregion
         QModelIndex index(int row, int column, const QModelIndex& parent) const override;
         QModelIndex parent(const QModelIndex& index) const;
         int rowCount(const QModelIndex& parent) const override;
         int columnCount(const QModelIndex& item) const override;
         Qt::ItemFlags flags(const QModelIndex& index) const override;
         QVariant data(const QModelIndex& index, int role) const override;
         QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
      #pragma endregion
      
   public slots:
      void buildAllQuestFilters();
      void clearAllQuestFilters();
      void prepForQuestFilterChange(const QString& filter); // takes the filter the quest used to have
      void finishQuestFilterChange(const QString& filter); // takes the filter the quest currently has, which may be the same one it used to have
};

class ObjectWindowTree : public QLinedTreeView {
   Q_OBJECT
   public:
      ObjectWindowTree(QWidget* parent);
      using model_type      = ObjectWindowTreeModel;
      using model_item_type = model_type::item_type;
      //
      QVector<dovah::form_type_t> selectedFormTypes() const noexcept;
};