#pragma once
#include <cstdint>
#include <QAbstractItemModel>
#include <QLineEdit>
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
      using bare_form_id_t = dovah::bare_form_id_t;
      using form_stub      = dovah::form_stub;
      using form_type_t    = dovah::form_type_t;
      //
      const form_stub* stub   = nullptr;
      const form_stub* base   = nullptr;
      bare_form_id_t   formID = 0;
      QString          editorID;
      //
      bool is_active   = false;
      bool is_injected = false;
      //
      CellRefListModelItem() {}
      CellRefListModelItem(const form_stub*);
      //
      form_type_t formType() const noexcept;
      void update();
};

class CellRefListModel : public QAbstractTableModel {
   Q_OBJECT
   public:
      using item_type = CellRefListModelItem;
      using form_stub = dovah::form_stub;
   protected:
      QVector<item_type*> children;
      QVector<item_type*> queued_additions;
      const form_stub* last_used_cell = nullptr;
      //
      void _insertItem(const form_stub*, bool queued);
      //
   public slots:
      void clear();
      //
   protected slots:
      void formCreated(const form_stub*);
      void formModified(const form_stub*);
      void formDeletionImminent(const dovah::form_stub*, bool is_just_flagged);
      void formRenumbered(const dovah::form_stub*, dovah::bare_form_id_t oldID, dovah::bare_form_id_t newID);
      //
   public:
      CellRefListModel(QObject* parent = nullptr);
      ~CellRefListModel() {
         this->clear();
      }
      //
      QModelIndex index(int row, int column, const QModelIndex& parent) const override;
      QModelIndex parent(const QModelIndex& index) const;
      int rowCount(const QModelIndex& parent) const override;
      int columnCount(const QModelIndex& item) const override;
      Qt::ItemFlags flags(const QModelIndex& index) const override;
      QVariant data(const QModelIndex& index, int role) const override;
      inline const item_type* row(int rowIndex) const noexcept;
      //
      QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
      //
      void insertItem(dovah::form_stub*);
      void rebuild(const dovah::form_stub* cell);
};

class CellRefListModelProxy : public QSortFilterProxyModel {
   Q_OBJECT
   public:
      CellRefListModelProxy(QObject* parent = nullptr);
      using model_type      = CellRefListModel;
      using model_item_type = model_type::item_type;
      //
      bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
      //
      inline dovah::form_type_t formType() const noexcept { return this->_formType; };
      void setFormType(dovah::form_type_t);
      //
   protected:
      dovah::form_type_t _formType = dovah::form_type::none;
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
      void setTextFilter(QLineEdit*);
      void setFormTypeFilter(dovah::form_type_t);
      //
      dovah::bare_form_id_t formID() const noexcept;
      dovah::form_stub* formStub() const noexcept;
      //
   public slots:
      void rebuildModel();
      void clear();
      void refilterModelByText(const QString&);
      void textFilterChanged();
      void textFilterFinished();
      //
   protected:
      const CellList* _cellSelector = nullptr;
      QLineEdit* _filter = nullptr;
      QTimer*    _filterThrottle = new QTimer(this);
      //
      model_item_type* _getCurrentItem() const noexcept;
};