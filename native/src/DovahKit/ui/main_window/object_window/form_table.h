#pragma once
#include <cstdint>
#include <set>
#include <vector>
#include <QAbstractItemModel>
#include <QString>
#include <QTableView>
#include "../../../dovah/core.h"

namespace dovah {
   class form_stub;
}
class BasicFormTypeTree;

class FormTableModel;
class FormTableModelItem {
   friend FormTableModel;
   public:
      using form_id_t = dovah::bare_form_id_t;
      //
      dovah::form_stub* stub = nullptr;
      QString   editorID;
      form_id_t formID    = 0;
      uint32_t  userCount = 0;
      //
      FormTableModelItem() {}
      FormTableModelItem(dovah::form_stub*);
      //
      inline const QString& name() const noexcept { return this->editorID; }
};
class FormTableModelRoot : public FormTableModelItem {
   friend FormTableModel;
   protected:
      std::vector<FormTableModelItem*> _children;
   public:
      inline const std::vector<FormTableModelItem*>& children() const noexcept { return this->_children; }
      inline FormTableModelItem* child(size_t i) const noexcept {
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

class FormTableModel : public QAbstractTableModel {
   Q_OBJECT
   public:
      using item_type = FormTableModelItem;
      using root_type = FormTableModelRoot;
      using form_id_t = item_type::form_id_t;
   protected:
      root_type*    root = nullptr;
      Qt::SortOrder last_sort_order;
      int           last_sort_column = -1;
      //
   public:
      FormTableModel() {
         this->root = new root_type;
      }
      ~FormTableModel() {
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
      void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;
      inline Qt::SortOrder lastSortOrder() const noexcept { return this->last_sort_order; }
      inline int lastSortColumn() const noexcept { return this->last_sort_column; }
      //
      void insertItem(dovah::form_stub*);
      //
      void clear();
};

class FormTable : public QTableView {
   Q_OBJECT
   public:
      FormTable(QWidget* parent);
      using model_type      = FormTableModel;
      using model_item_type = model_type::item_type;
      using form_type_set   = std::set<dovah::form_type_t>;
      //
      void setSource(BasicFormTypeTree*);
      //
   public slots:
      void recheckFormTypes();
      void rebuildModel();
      //
   protected:
      BasicFormTypeTree* _source = nullptr;
      form_type_set _currentFormTypes;
      //
      void _adaptSourceSelectionChange(const QItemSelection& selected, const QItemSelection& deselected); // QItemSelectionModel's selection events all fire BEFORE the widget's selection is updated UGHHHHH
};