#pragma once
#include <cstdint>
#include <QAbstractItemModel>
#include <QDateTime>
#include <QSortFilterProxyModel>
#include <QString>
#include <QTableView>
#include <QTimer>
#include "../../../dovah/bare_form_id_t.h"
#include "../../../dovah/form_stub.h"
#include "../../../dovah/data/default_objects.h"

namespace dovah::loaded_forms {
   class DefaultObjectManager;
}

class DefaultObjectListModel;
class DefaultObjectListModelItem {
   //
   // Given a model which displays all users of a form, this item represents a 
   // user form (as opposed to the used form).
   //
   friend DefaultObjectListModel;
   public:
      using bare_form_id_t = dovah::bare_form_id_t;
      //
      uint32_t signature = 0;
      QString  name;
      QString  description;
      const dovah::form_stub* form      = nullptr;
      dovah::form_type        form_type = dovah::form_type::none;
      bool is_edited = false;
      //
      DefaultObjectListModelItem() {}
      DefaultObjectListModelItem(uint32_t signature, dovah::form_type ft);
      //
      QString valueAsString() const noexcept;
};

class DefaultObjectListModel : public QAbstractTableModel {
   Q_OBJECT
   public:
      using item_type = DefaultObjectListModelItem;
      static constexpr int ColumnSignature = 0;
      static constexpr int ColumnName      = 1;
      static constexpr int ColumnValue     = 2;
      static constexpr Qt::ItemDataRole SortRole   = (Qt::ItemDataRole)(Qt::UserRole + 0);
      static constexpr Qt::ItemDataRole FilterRole = (Qt::ItemDataRole)(Qt::UserRole + 1);
      //
   protected:
      QVector<item_type*> children;
      QVector<item_type*> queued_additions;
      //
      using form_t     = dovah::loaded_forms::DefaultObjectManager;
      using form_ptr_t = dovah::loaded_form_ptr<form_t>;
      //
   protected slots:
      void defaultObjectEntryChanged(uint32_t signature);
      void formModified(const dovah::form_stub*);
      void formDeletionImminent(const dovah::form_stub*, bool is_just_flagged);
      void formRenumbered(const dovah::form_stub*, dovah::bare_form_id_t oldID, dovah::bare_form_id_t newID);
      //
   public:
      DefaultObjectListModel(QObject* parent = nullptr);
      ~DefaultObjectListModel() {
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
      void build();
      //
   public slots:
      void clear();
};

class DefaultObjectListModelProxy : public QSortFilterProxyModel {
   Q_OBJECT
   public:
      DefaultObjectListModelProxy(QObject* parent = nullptr);
      using model_type      = DefaultObjectListModel;
      using model_item_type = model_type::item_type;
};

class DefaultObjectList : public QTableView {
   Q_OBJECT
   public:
      DefaultObjectList(QWidget* parent);
      using model_type        = DefaultObjectListModel;
      using model_item_type   = model_type::item_type;
      //
      inline model_type* unwrappedModel() const noexcept {
         auto wrapper = (QSortFilterProxyModel*)this->model();
         return wrapper ? (model_type*)wrapper->sourceModel() : nullptr;
      }
      void setTextFilter(QLineEdit*);
      void setFormTypeFilter(dovah::form_type);
      //
   public slots:
      void build();
      void refilterModelByText(const QString&);
      void textFilterChanged();
      void textFilterFinished();
      //
   protected:
      QLineEdit* _filter         = nullptr;
      QTimer*    _filterThrottle = new QTimer(this);
};