#pragma once
#include <array>
#include <cstdint>
#include <vector>
#include <QAbstractItemModel>
#include <QString>
#include <QTableView>
#include "../../../dovah/core.h"
#include "../../../dovah/forms/components/conditions.h"

namespace dovah {
   class form_stub;
}

class ConditionListModel : public QAbstractTableModel {
   Q_OBJECT
   public:
      using form_stub = dovah::form_stub;
      using condition = dovah::loaded_forms::components::condition;
      using loaded_form_t = dovah::loaded_forms::Form;
      using arg_underlying_type = dovah::loaded_forms::components::condition_info::arg_underlying_type;
      using condition_function  = dovah::loaded_forms::components::condition_info::function;
      //
      static constexpr int ColumnTarget   = 0;
      static constexpr int ColumnFunction = 1;
      static constexpr int ColumnArgs     = 2; // comma-separated args
      static constexpr int ColumnOperator = 3;
      static constexpr int ColumnOperand  = 4;
      static constexpr int ColumnUsesOr   = 5;
      //
   protected:
      form_stub*     owner = nullptr; // form which owns the condition list (if a normal form)
      loaded_form_t* clone = nullptr; // form which owns the condition list (if a temporary/working copy)
      std::vector<condition>* target = nullptr; // condition list to modify
      //
   protected slots:
      void formModified(const dovah::form_stub*);
      void formDeletionImminent(const dovah::form_stub*, bool is_just_flagged);
      //
   public slots:
      void clear();
      //
   public:
      ConditionListModel(QObject* parent = nullptr);
      ~ConditionListModel() {
         this->clear();
      }
      //
      QModelIndex index(int row, int column, const QModelIndex& parent) const override;
      QModelIndex parent(const QModelIndex& index) const;
      int rowCount(const QModelIndex& parent) const override;
      int columnCount(const QModelIndex& item) const override;
      Qt::ItemFlags flags(const QModelIndex& index) const override;
      QVariant data(const QModelIndex& index, int role) const override;
      //
      QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
};

class ConditionList : public QTableView {
   Q_OBJECT
   public:
      ConditionList(QWidget* parent);
      using model_type = ConditionListModel;
      //
      inline model_type* fullModel() const noexcept {
         return (model_type*)this->model();
      }
      //
   public slots:
};