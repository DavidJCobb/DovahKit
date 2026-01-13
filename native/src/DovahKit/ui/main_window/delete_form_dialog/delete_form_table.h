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
#include "../../../dovah/files/file_load_order.h"

class DeleteFormDialogListModel;
class DeleteFormDialogListModelItem {
   friend DeleteFormDialogListModel;
   public:
      using bare_form_id_t = dovah::bare_form_id_t;
      using form_stub      = dovah::form_stub;
      using form_type      = dovah::form_type;
      //
      const form_stub& stub;
      bare_form_id_t   formID = 0;
      QString          editorID;
      QString          signature;
      QString          parentCell;
      //
      DeleteFormDialogListModelItem(const form_stub&);
      void updateFromStub(); // update the form's identifying information, e.g. its editor ID
};

class DeleteFormDialogListModel : public QAbstractTableModel {
   Q_OBJECT
   public:
      using item_type = DeleteFormDialogListModelItem;
      using form_stub = dovah::form_stub;
   protected:
      QVector<item_type*> children;
      QVector<item_type*> queued_additions;
      //
   protected slots:
      void formModified(const dovah::form_stub*);
      void formDeletionImminent(const dovah::form_stub*, bool is_just_flagged);
      void formRenumbered(const dovah::form_stub*, dovah::bare_form_id_t oldID, dovah::bare_form_id_t newID);
      //
   public:
      DeleteFormDialogListModel(QObject* parent = nullptr);
      ~DeleteFormDialogListModel() {
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
      bool contains(const dovah::form_stub&) const noexcept;
      //
   public slots:
      void clear();
      void insert(const dovah::form_stub&, bool queued = false);
      void remove(const dovah::form_stub&);
      void commitQueuedAdditions();
      //
      void insertDeletions(const dovah::form_deletion_request&);
      void insertUsers(const dovah::form_deletion_request&);
};

class DeleteFormDialogListModelProxy : public QSortFilterProxyModel {
   Q_OBJECT
   public:
      DeleteFormDialogListModelProxy(QObject* parent = nullptr);
      using model_type      = DeleteFormDialogListModel;
      using model_item_type = model_type::item_type;
};

class DeleteFormDialogList : public QTableView {
   Q_OBJECT
   public:
      DeleteFormDialogList(QWidget* parent);
      using model_type      = DeleteFormDialogListModel;
      using model_item_type = model_type::item_type;
      //
      inline model_type* unwrappedModel() const noexcept {
         auto wrapper = (QSortFilterProxyModel*)this->model();
         return wrapper ? (model_type*)wrapper->sourceModel() : nullptr;
      }
};