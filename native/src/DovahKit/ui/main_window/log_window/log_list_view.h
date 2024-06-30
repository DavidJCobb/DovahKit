#pragma once
#include <cstdint>
#include <QAbstractItemModel>
#include <QIcon>
#include <QString>
#include "dovah/core.h"
#include "ui/types/logging/log_item.h"

namespace dovah::notices {
   class base_error;
   class base_warning;
}

class LogListModel : public QAbstractTableModel {
   Q_OBJECT
   public:
      using item_type = ui::types::log_item;

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
      void dataAbandonImminent();
      void dataSaveImminent();
      void dataSaveComplete();
      void formRenumbered(dovah::form_stub*, dovah::bare_form_id_t, dovah::bare_form_id_t);
      void formDeletionImminent(dovah::form_stub*, bool);

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

   protected:
      void _appendLogItem(item_type*); // takes ownership

      template<typename... Args>
      void _createLogItem(Args&&... args) {
         this->_appendLogItem(new item_type(std::forward<Args>(args)...));
      }
      
   public slots:
      void addLogItem(item_type&&);
      void addLogItem(const item_type&);
      void clear();
};