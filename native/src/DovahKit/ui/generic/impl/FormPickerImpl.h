#pragma once
#include "../../../dovah/core.h"
#include "../../../editor/core.h"
#include <QAbstractItemModel>
#include <QAbstractProxyModel>
#include <QSortFilterProxyModel>
#include <QTimer>

namespace FormPickerImpl {
   class FormPickerSharedUnderlyingModel : public QAbstractItemModel {
      Q_OBJECT;
      public:
         static constexpr int FormIDRole   = Qt::UserRole + 0;
         static constexpr int FormStubRole = Qt::UserRole + 1;

         struct item {
            dovah::form_stub*  stub = nullptr;
            dovah::form_type_t type = dovah::form_type::none;
            QString editorID;
         };

      protected:
         FormPickerSharedUnderlyingModel();

         QVector<item*> forms; // always include a "NONE" item

      protected slots:
         void rebuild();

      public:
         static FormPickerSharedUnderlyingModel& get() {
            static FormPickerSharedUnderlyingModel instance;
            return instance;
         }

         QModelIndex index(int row, int column, const QModelIndex& parent) const override;
         QModelIndex parent(const QModelIndex& index) const;
         int rowCount(const QModelIndex& parent) const override;
         int columnCount(const QModelIndex& item) const override;
         Qt::ItemFlags flags(const QModelIndex& index) const override;
         QVariant data(const QModelIndex& index, int role) const override;

         const item* itemAtRow(int) const noexcept;

      signals:
         void allDataCleared();
         void editorIDChanged(const item*, const QString& prior);
   };

   class FormPickerIterativeModel : public QAbstractItemModel {
      Q_OBJECT;
      public:
         using item      = FormPickerSharedUnderlyingModel::item;
         using item_list = QVector<const item*>;
         using iterator  = item_list::iterator;
      protected:
         item_list stubs;
         struct {
            bool filling    = true;
            bool sorting    = false;
            int  progress   = 0;
            bool allow_none = false;
            QVector<dovah::form_type_t> form_types;
            item_list unsorted;
            QTimer timer;
         } ongoing_fill;

         inline bool _isFilling() const noexcept { return this->ongoing_fill.filling; }

         bool _fillGrabMore(); // returns true if done
         bool _fillSortMore(); // returns true if done

      public:
         FormPickerIterativeModel(QObject* parent);

         virtual void fetchMore(const QModelIndex& parent) override;
         virtual bool canFetchMore(const QModelIndex& parent) const override;

         virtual QModelIndex index(int row, int column, const QModelIndex& parent) const override;
         virtual QModelIndex parent(const QModelIndex& index) const override;
         virtual int rowCount(const QModelIndex& parent) const override;
         virtual int columnCount(const QModelIndex& item) const override;
         virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
         virtual QVariant data(const QModelIndex& index, int role) const override;

         void refill(bool allow_none, const QVector<dovah::form_type_t>& form_types);
         void updateParameters(bool allow_none, const QVector<dovah::form_type_t>& form_types);

         int indexOf(const dovah::form_stub*) const noexcept;

      signals:
         void filled();
   };
}