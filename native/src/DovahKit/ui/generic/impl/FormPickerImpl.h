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
      protected:
         FormPickerSharedUnderlyingModel();

         QVector<dovah::form_stub*> all_stubs = { nullptr }; // always include a "NONE" item

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

         inline const QVector<dovah::form_stub*>& getRawStubList() const noexcept { return this->all_stubs; }
   };

   class FormPickerIterativeModel : public QAbstractItemModel {
      protected:
         QVector<dovah::form_stub*> stubs;
         struct {
            bool filling    = true;
            int  progress   = 0;
            bool allow_none = false;
            QVector<dovah::form_type_t> form_types;
            QTimer timer;
         } ongoing_fill;

         inline bool _isFilling() const noexcept { return this->ongoing_fill.filling; }

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
   };

   class FormPickerProxyModel : public QSortFilterProxyModel {
      Q_OBJECT;
      protected:
         bool _allowNone = true;
         QVector<dovah::form_type_t> _formTypes;
      public:
         FormPickerProxyModel(QObject* parent = nullptr);

         inline bool allowNone() const noexcept { return this->_allowNone; }
         inline const QVector<dovah::form_type_t>& allowedFormTypes() const noexcept { return this->_formTypes; }

         void setAllowedFormTypes(const QVector<dovah::form_type_t>&);
         void setAllowNone(bool);
         void updateParameters(bool allow_none, const QVector<dovah::form_type_t>& form_types);

         inline bool isEmpty() const noexcept { return this->rowCount() > 0; }

         virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;
         virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;
         virtual void setSourceModel(QAbstractItemModel* sourceModel) override {}
   };
}