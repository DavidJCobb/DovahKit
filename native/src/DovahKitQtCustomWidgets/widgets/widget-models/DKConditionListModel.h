#pragma once
#if defined(QT_PLUGIN)
   #error This model relies on DovahKit to run. Do not include it when compiling the Qt Designer plug-in.
#endif
#include <QAbstractTableModel>
#include <QItemSelectionModel>
#include "ui/models/DKGenericListModel.h"
#include "ui/types/conditions/condition.h"

namespace dovah::loaded_forms {
   class Form;
}

class DKConditionListModel;
class DKConditionListModel : public DKGenericListModel<DKConditionListModel, ui::types::conditions::condition> {
   Q_OBJECT;
   friend DKGenericListModel; // needed for CRTP machinery
   public:
      struct Column {
         Column() = delete;
         enum {
            Target,
            Function,
            Args,     // shown as comma-separated
            Operator,
            Operand,
            UsesOr,
            _COUNT
         };
      };
      static constexpr const size_t column_count = Column::_COUNT;

      using BackendCondition     = dovah::loaded_forms::components::condition;
      using BackendConditionList = dovah::loaded_forms::components::condition_list;

      using Condition = node_type;
      
   protected:
      ui::types::conditions::context _context;

      // For bifurcated condition lists -- that is, edge-cases where a form ends up accidentally 
      // coalescing conditions across multiple records/overrides, such that the first N conditions 
      // come from a master and the conditions after that come from the latest file (potentially 
      // the active file).
      size_t _allow_modifications_from = 0;
      
   public:
      DKConditionListModel(QObject* parent = nullptr);
      ~DKConditionListModel();

   protected:
      QString _stringify_condition_parameter(const Condition&, size_t i) const;

      QVariant      data_of(const node_type&, Qt::ItemDataRole, size_t column) const;
      Qt::ItemFlags flags_of(const node_type&, size_t column) const;
      
   protected slots:
      void formModified(const dovah::form_stub*);
      void formDeletionImminent(const dovah::form_stub*, bool is_just_flagged);
      
   public slots:
      void clear();

   public:
      virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
      
   public:
      [[nodiscard]] const std::vector<Condition> conditions() const noexcept;
      inline const size_t conditionCount() const noexcept {
         return this->_nodes.size();
      }
      void duplicate(const QItemSelection&);
      QModelIndex insertAt(const Condition&, size_t at);
      void move(const QItemSelection&, int down);
      void remove(const QItemSelection&);

      const Condition* getCondition(size_t row) const;
      void setCondition(size_t row, const Condition&);

      // Returns number of invalid conditions discarded.
      size_t importFrom(dovah::loaded_forms::Form&, const BackendConditionList&);
      size_t importFrom(dovah::loaded_forms::Form&, const std::vector<Condition>&);

      size_t importBifurcatedList(dovah::loaded_forms::Form&, const BackendConditionList& locked, const BackendConditionList& normal);
      void exportBifurcatedList(dovah::loaded_forms::Form&, BackendConditionList& locked, BackendConditionList& normal);
};