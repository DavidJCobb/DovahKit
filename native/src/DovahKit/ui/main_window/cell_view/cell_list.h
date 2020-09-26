#pragma once
#include <cstdint>
#include <vector>
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QString>
#include <QTableView>
#include <QTimer>
#include "../../../dovah/core.h"
#include "../../generic/FormsOfTypeCombobox.h"

namespace dovah {
   class form_stub;
}
class BasicFormTypeTree;

class CellListModel;
class CellListModelItem {
   friend CellListModel;
   public:
      using form_id_t = dovah::bare_form_id_t;
      //
      dovah::form_stub* stub = nullptr;
      QString   editorID;
      form_id_t formID    = 0;
      int32_t   gridX     = 0;
      int32_t   gridY     = 0;
      //
      CellListModelItem() {}
      CellListModelItem(dovah::form_stub*);
      //
      bool cellIsLoaded();
      void update();
};
class CellListModelRoot : public CellListModelItem {
   friend CellListModel;
   protected:
      std::vector<CellListModelItem*> _children;
   public:
      inline const std::vector<CellListModelItem*>& children() const noexcept { return this->_children; }
      inline CellListModelItem* child(size_t i) const noexcept {
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

class CellListModel : public QAbstractTableModel {
   Q_OBJECT
   public:
      using item_type = CellListModelItem;
      using root_type = CellListModelRoot;
      using form_id_t = item_type::form_id_t;
   protected:
      root_type* root = nullptr;
      //
   public:
      CellListModel() {
         this->root = new root_type;
      }
      ~CellListModel() {
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
      void rebuild(const dovah::form_stub* worldspace);
};

class CellListModelProxy : public QSortFilterProxyModel {
   Q_OBJECT
   public:
      CellListModelProxy(QObject* parent = nullptr) : QSortFilterProxyModel(parent) {
         this->setFilterCaseSensitivity(Qt::CaseInsensitive);
         this->setFilterRole(Qt::UserRole + 1);
         this->setFilterKeyColumn(-1);
         this->setSortCaseSensitivity(Qt::CaseInsensitive);
         this->setSortRole(Qt::UserRole);
      }
};

class CellList : public QTableView {
   Q_OBJECT
   public:
      CellList(QWidget* parent);
      using proxy_type      = CellListModelProxy;
      using model_type      = CellListModel;
      using model_item_type = model_type::item_type;
      //
      inline model_type* unwrappedModel() const noexcept {
         auto wrapper = (QSortFilterProxyModel*)this->model();
         return wrapper ? (model_type*)wrapper->sourceModel() : nullptr;
      }
      //
      inline const FormsOfTypeCombobox* worldspacePicker() const noexcept { return this->_worldspaceSelector; }
      void setWorldspacePicker(const FormsOfTypeCombobox*);
      //
      dovah::bare_form_id_t formID() const noexcept;
      dovah::form_stub* formStub() const noexcept;
      //
   public slots:
      void rebuildModel();
      void clear();
      //
   signals:
      void currentCellChanged(const dovah::form_stub* cell);
      //
   protected:
      const FormsOfTypeCombobox* _worldspaceSelector = nullptr;
      //
      model_item_type* _getCurrentItem() const noexcept;
};