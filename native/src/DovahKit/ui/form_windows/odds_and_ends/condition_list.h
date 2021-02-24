#pragma once
#include <array>
#include <cstdint>
#include <vector>
#include <QAbstractItemModel>
#include <QString>
#include <QTableView>
#include "../../../dovah/core.h"
#include "../../../dovah/data/conditions.h"
#include "../../../dovah/forms/components/conditions.h"
#include "ui_condition_list.h"

namespace dovah {
   class form_stub;
   namespace loaded_forms {
      class Form;
      class Package;
      class Quest;
   }
}

class ConditionListModel : public QAbstractTableModel {
   Q_OBJECT
   public:
      using form_stub = dovah::form_stub;
      using condition = dovah::loaded_forms::components::condition;
      using cnd_context_t = dovah::loaded_forms::components::condition_context;
      using arg_underlying_type = dovah::condition_parameter_underlying_type;
      using condition_function  = dovah::condition_function;
      //
      static constexpr int ColumnTarget   = 0;
      static constexpr int ColumnFunction = 1;
      static constexpr int ColumnArgs     = 2; // comma-separated args
      static constexpr int ColumnOperator = 3;
      static constexpr int ColumnOperand  = 4;
      static constexpr int ColumnUsesOr   = 5;
      //
   protected:
      cnd_context_t context;
      std::vector<condition>* target = nullptr; // condition list to modify
      bool in_working_copy = true;
      //
   protected slots:
      void formModified(const dovah::form_stub*);
      void formDeletionImminent(const dovah::form_stub*, bool is_just_flagged);
      //
   public slots:
      void clearTarget();
      //
   public:
      ConditionListModel(QObject* parent = nullptr);
      ~ConditionListModel() {
         this->clearTarget();
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

      //
      // don't call this directly; Qt's design for this API is unintuitive; you WILL screw up
      //
      // use (moveSelection) instead
      //
      virtual bool moveRows(const QModelIndex& from_parent, int first_row_index, int count, const QModelIndex& to_parent, int to_position) override;
      
      inline int count() const noexcept {
         if (!this->target)
            return 0;
         return this->target->size();
      }
      void duplicateSelection(const QItemSelection&);
      QModelIndex insertCondition(const dovah::loaded_forms::components::working_condition&, size_t at);
      void moveSelection(const QItemSelection&, int down);
      void refresh();
      void removeSelection(const QItemSelection&);
      void setTarget(form_stub&, std::vector<condition>&, bool in_working_copy = true);

      form_stub* targetStub() const noexcept { return this->context.owner; }
      dovah::loaded_forms::Form* targetLoadedForm() const noexcept;
      condition* getCondition(const QModelIndex&);
};

class ConditionList : public QWidget {
   Q_OBJECT
   public:
      ConditionList(QWidget* parent);
      using model_type = ConditionListModel;
      //
      model_type* model() const noexcept;
      //
   public slots:
      void openCreateConditionModal();
      void openEditConditionModal();
      //
   signals:
      void conditionEdited();
      //
   protected:
      Ui::ConditionList ui;
      struct {
         QAction* create    = nullptr;
         QAction* edit      = nullptr;
         QAction* duplicate = nullptr;
         QAction* destroy   = nullptr;
      } context_menu_actions;
};