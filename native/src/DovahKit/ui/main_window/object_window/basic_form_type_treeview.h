#pragma once
#include <cstdint>
#include <vector>
#include <QAbstractItemModel>
#include <QString>
#include <QTreeView>
#include "../../../dovah/core.h"
#include "../../generic/QLinedTreeView.h"

class BasicFormTypeTreeModel;
class BasicFormTypeTreeModelItem { // represents a (sub)category
   friend BasicFormTypeTreeModel;
   public:
      static constexpr int no_filter = -1;
      //
      const int  form_type = no_filter;
      const bool is_filter = false;
   protected:
      const QString _name;
      BasicFormTypeTreeModelItem* _parent = nullptr;
      std::vector<BasicFormTypeTreeModelItem*> _children;
      int _filter_refcount = 0;
      //
      void _destroyDescendants() noexcept;
      //
   public:
      BasicFormTypeTreeModelItem(const QString& n, int ft) : _name(n), form_type(ft) {}
      BasicFormTypeTreeModelItem(const QString& n) : _name(n), is_filter(true) {}
      //
      void appendChild(BasicFormTypeTreeModelItem*) noexcept;
      void removeChild(BasicFormTypeTreeModelItem*) noexcept;
      inline const std::vector<BasicFormTypeTreeModelItem*>& children() const noexcept { return this->_children; }
      inline BasicFormTypeTreeModelItem* child(size_t i) const noexcept {
         if (i < 0 || i >= this->_children.size())
            return nullptr;
         return this->_children[i];
      }
      inline size_t childCount() const noexcept { return this->_children.size(); }
      int32_t indexOf(BasicFormTypeTreeModelItem*) const noexcept;
      inline const QString& name() const noexcept { return this->_name; }
      inline BasicFormTypeTreeModelItem* parent() const noexcept { return this->_parent; }

      void addToSet(QVector<dovah::form_type_t>& out) const noexcept;
};

class BasicFormTypeTreeModel : public QAbstractItemModel {
   Q_OBJECT
   public:
      using item_type = BasicFormTypeTreeModelItem;
   protected:
      item_type* root = nullptr;
      //
   public:
      BasicFormTypeTreeModel(QObject* parent = nullptr);
      ~BasicFormTypeTreeModel() {
         this->clear();
      }
      //
      QModelIndex index(int row, int column, const QModelIndex& parent) const override;
      inline item_type* invisibleRootItem() const noexcept { return this->root; }
      QModelIndex parent(const QModelIndex& index) const;
      int rowCount(const QModelIndex& parent) const override;
      int columnCount(const QModelIndex& item) const override;
      Qt::ItemFlags flags(const QModelIndex& index) const override;
      QVariant data(const QModelIndex& index, int role) const override;
      //
      QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
      //
      void clear();
      //
   public slots:
      void rebuildQuestFilters();
};

class BasicFormTypeTree : public QLinedTreeView {
   Q_OBJECT
   public:
      BasicFormTypeTree(QWidget* parent);
      using model_type      = BasicFormTypeTreeModel;
      using model_item_type = model_type::item_type;
      //
      QVector<dovah::form_type_t> selectedFormTypes() const noexcept;
};