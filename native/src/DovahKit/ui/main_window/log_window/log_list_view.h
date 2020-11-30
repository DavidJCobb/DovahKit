#pragma once
#include <cstdint>
#include <QAbstractItemModel>
#include <QListView>
#include <QString>
#include "../../../dovah/core.h"
#include "../../../dovah/detailed_notice.h"
#include "../../../dovah/files/file_read_warning.h"
#include "../../../dovah/files/file_write_error.h"

class LogListModel;
class LogListModelItem {
   //
   // Given a model which displays all users of a form, this item represents a 
   // user form (as opposed to the used form).
   //
   friend LogListModel;
   public:
      using detailed_notice = dovah::detailed_notice;
      using file_read_warning = dovah::file_read_warning;
      enum class type_t {
         text,
         file_read_warning, // (text) stores the "rendered" output
      };
      //
      type_t  type = type_t::text;
      QString text;
      QString file;
      file_read_warning data;
      //
      LogListModelItem() {}
      LogListModelItem(const QString&);
      LogListModelItem(const file_read_warning&);
      LogListModelItem(const detailed_notice&);
      //
      bool compare(const file_read_warning&) const noexcept;
      bool empty() const noexcept;
};

class LogListModel : public QAbstractTableModel {
   Q_OBJECT
   public:
      using item_type = LogListModelItem;
   protected:
      QVector<item_type*> children;
      QHash<dovah::bare_form_id_t, QVector<item_type*>> warnings_cause_by_form; // used to avoid showing duplicate warnings for on-demand form loads
      //
   protected slots:
      void dataSaveImminent();
      void dataSaveComplete();
      void saveErrorReceived(const dovah::detailed_notice& error);
      void loadWarningReceived(const dovah::file_read_warning& warning);
      void gameSettingValueChangeFailed(const char* name, dovah::notice_code_t);
      //
   public:
      LogListModel(QObject* parent = nullptr);
      ~LogListModel() {
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
   public slots:
      void addTextEntry(const QString&);
      void clear();
};

class LogList : public QListView {
   Q_OBJECT
   public:
      LogList(QWidget* parent);
      using model_type      = LogListModel;
      using model_item_type = model_type::item_type;
};