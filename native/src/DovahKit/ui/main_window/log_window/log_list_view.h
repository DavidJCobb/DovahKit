#pragma once
#include <cstdint>
#include <QAbstractItemModel>
#include <QIcon>
#include <QString>
#include <QTableView>
#include "dovah/core.h"

namespace dovah::notices {
   class base_error;
   class base_warning;
}

class LogListModel;
class LogListModelItem {
   //
   // Given a model which displays all users of a form, this item represents a 
   // user form (as opposed to the used form).
   //
   friend LogListModel;
   public:
      enum class Type {
         Unspecified,
         Warning,
         Error,
      };
      enum class Context {
         Unspecified,
         FileLoad,
         FormLoad,
         FormSave,
         FileSave,
      };

   public:
      enum class type_t {
         text,
      };
      
      struct {
         Type    type    = Type::Unspecified;
         Context context = Context::Unspecified;
      } metadata;
      type_t  type = type_t::text;
      QString text;
      QString file;
      
      LogListModelItem() {}
      LogListModelItem(const QString&);
      LogListModelItem(const dovah::notices::base_error&);
      LogListModelItem(const dovah::notices::base_warning&);
      
      bool empty() const noexcept;
};

class LogListModel : public QAbstractTableModel {
   Q_OBJECT
   public:
      using item_type = LogListModelItem;

      struct Column { // scoped loose enum
         Column() = delete;
         enum {
            Context,
            Type,
            Text,
            File,
            _COUNT
         };
      };
      static constexpr const size_t ColumnCount = Column::_COUNT;

   protected:
      QVector<item_type*> children;
      QHash<dovah::bare_form_id_t, QVector<item_type*>> warnings_cause_by_form; // used to avoid showing duplicate warnings for on-demand form loads
      struct {
         struct {
            QIcon file_load;
            QIcon file_save;
            QIcon form_load;
         } contexts;
         QIcon error;
         QIcon warning;
      } _icons;
      
      bool _has_matching_notice(dovah::bare_form_id_t, QString text) const;

   protected slots:
      void dataAcquireComplete();
      void dataSaveImminent();
      void dataSaveComplete();

      void errorReceived(const dovah::notices::base_error&);
      void warningReceived(const dovah::notices::base_warning&);
      
   public:
      LogListModel(QObject* parent = nullptr);
      ~LogListModel() {
         this->clear();
      }
      
      QModelIndex index(int row, int column, const QModelIndex& parent) const override;
      QModelIndex parent(const QModelIndex& index) const;
      int rowCount(const QModelIndex& parent) const override;
      int columnCount(const QModelIndex& item) const override;
      Qt::ItemFlags flags(const QModelIndex& index) const override;
      QVariant data(const QModelIndex& index, int role) const override;
      inline const item_type* row(int rowIndex) const noexcept;
      
      QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
      
   public slots:
      void addTextEntry(const QString&, LogListModelItem::Type type = LogListModelItem::Type::Unspecified, LogListModelItem::Context context = LogListModelItem::Context::Unspecified);
      void clear();
};

class LogList : public QTableView {
   Q_OBJECT
   public:
      LogList(QWidget* parent);
      using model_type      = LogListModel;
      using model_item_type = model_type::item_type;
};