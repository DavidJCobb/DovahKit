#pragma once
#include <cstdint>
#include <vector>
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QString>
#include <QTableView>
#include <QTimer>
#include "../../../dovah/core.h"
#include "cell_list.h"

namespace dovah {
   class form_stub;
}
class BasicFormTypeTree;

class CellRefListModel;
class CellRefListModelItem {
   friend CellRefListModel;
   public:
      using form_id_t   = dovah::bare_form_id_t;
      //
      dovah::form_stub* stub = nullptr;
      dovah::form_stub* base = nullptr;
      QString     editorID;
      form_id_t   formID = 0;
      //
      CellRefListModelItem() {}
      CellRefListModelItem(dovah::form_stub*);
      //
      dovah::form_type_t formType() const noexcept;
      void update();
};
class CellRefListModelRoot : public CellRefListModelItem {
   friend CellRefListModel;
   protected:
      std::vector<CellRefListModelItem*> _children;
   public:
      inline const std::vector<CellRefListModelItem*>& children() const noexcept { return this->_children; }
      inline CellRefListModelItem* child(size_t i) const noexcept {
         if (i < 0 || i >= this->_children.size())
            return nullptr;
         return this->_children[i];
      }
      inline size_t childCount() const noexcept { return this->_children.size(); }
      //
      void clear() {
         for (auto* p : this->_children)
            delete p;
         this->_children.clear();
      }
};

class CellRefListModel : public QAbstractTableModel {
   Q_OBJECT
   public:
      using item_type = CellRefListModelItem;
      using root_type = CellRefListModelRoot;
      using form_id_t = item_type::form_id_t;
   protected:
      root_type* root = nullptr;
      const dovah::form_stub* last_used_cell = nullptr;
      //
   public:
      CellRefListModel() {
         this->root = new root_type;
      }
      ~CellRefListModel() {
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
      void insertItem(dovah::form_stub*);
      void updateExistingItem(const dovah::form_stub*);
      //
      void clear();
      //
      void rebuild(const dovah::form_stub* cell);
};

class CellRefListModelProxy : public QSortFilterProxyModel {
   Q_OBJECT
   public:
      CellRefListModelProxy(QObject* parent = nullptr) : QSortFilterProxyModel(parent) {
         this->setFilterCaseSensitivity(Qt::CaseInsensitive);
         this->setFilterRole(Qt::UserRole + 1);
         this->setFilterKeyColumn(-1);
         this->setSortCaseSensitivity(Qt::CaseInsensitive);
         this->setSortRole(Qt::UserRole);
      }
};

class CellRefList : public QTableView {
   Q_OBJECT
   public:
      CellRefList(QWidget* parent);
      using proxy_type      = CellRefListModelProxy;
      using model_type      = CellRefListModel;
      using model_item_type = model_type::item_type;
      //
      inline model_type* unwrappedModel() const noexcept {
         auto wrapper = (QSortFilterProxyModel*)this->model();
         return wrapper ? (model_type*)wrapper->sourceModel() : nullptr;
      }
      //
      inline const CellList* cellPicker() const noexcept { return this->_cellSelector; }
      void setCellPicker(const CellList*);
      //
   public slots:
      void rebuildModel();
      void clear();
      //
   protected:
      const CellList* _cellSelector = nullptr;
      //
      model_item_type* _getCurrentItem() const noexcept;
};