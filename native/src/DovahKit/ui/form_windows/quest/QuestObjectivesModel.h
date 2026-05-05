#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include <QAbstractItemModel>
#include <QPointer>
#include "ui/types/conditions/condition.h"
namespace dovah::loaded_forms {
   class Quest;
}
class QuestAliasesModel;

class QuestObjectivesModel : public QAbstractItemModel {
   Q_OBJECT;
   public:
      struct Column {
         Column() = delete;
         enum {
            Index,
            IsOr,
            Text,

            __COUNT
         };
      };
      static constexpr const size_t ColumnCount = Column::__COUNT;

      struct TargetColumn {
         TargetColumn() = delete;
         enum {
            AliasName,
            Conditions,

            __COUNT
         };
      };
      static constexpr const size_t TargetColumnCount = TargetColumn::__COUNT;

      // Objective roles
      static constexpr const Qt::ItemDataRole IndexRole = (Qt::ItemDataRole)(Qt::UserRole + 0);
      static constexpr const Qt::ItemDataRole IsOrRole  = (Qt::ItemDataRole)(Qt::UserRole + 1);
      static constexpr const Qt::ItemDataRole TextRole  = (Qt::ItemDataRole)(Qt::UserRole + 2);

      // Target roles
      static constexpr const Qt::ItemDataRole TargetAliasIDRole     = (Qt::ItemDataRole)(Qt::UserRole + 10);
      static constexpr const Qt::ItemDataRole TargetIgnoreLocksRole = (Qt::ItemDataRole)(Qt::UserRole + 11);

      using condition_list   = std::vector<ui::types::conditions::condition>;
      using loaded_form_data = dovah::loaded_forms::Quest;

      // Use as the root index of a `QuestObjectiveTargetsModel` when no objective is selected.
      QModelIndex noObjectiveQMI() const noexcept;

   protected:
      struct Target {
         int32_t        alias_id = -1;
         bool           compass_markers_ignore_locks = false;
         condition_list conditions;
         struct {
            QString alias_name;
            QString conditions;
         } cached;
      };
      struct Objective {
         uint32_t index        = 0;
         bool     or_with_prev = false;
         QString  text;
         std::vector<std::unique_ptr<Target>> targets;
      };

   protected:
      #pragma region QMI<->pointer utils
         static bool _is_no_objective_qmi(const QModelIndex&);
         static bool _is_objective_qmi(const QModelIndex&);
         static bool _is_target_qmi(const QModelIndex&);

         const Objective* _qmi_to_objective(const QModelIndex&) const;
         Objective* _qmi_to_objective(const QModelIndex&);
         QModelIndex _objective_to_qmi(const Objective&, unsigned int col = 0) const;

         const Target* _qmi_to_target(const QModelIndex&) const;
         Target* _qmi_to_target(const QModelIndex&);
         QModelIndex _target_to_qmi(const Objective&, const Target&, unsigned int col = 0) const;

         const Objective* _parent_objective_for_target_qmi(const QModelIndex&) const;
         Objective* _parent_objective_for_target_qmi(const QModelIndex&);
      #pragma endregion

   public:
      QuestObjectivesModel(QObject* parent = nullptr);

      void load(const loaded_form_data&);
      void save(loaded_form_data&);
      void setAliasesModel(const QuestAliasesModel*);

      #pragma region QAbstractItemModel overrides
         #pragma region Hierarchy
            virtual QModelIndex index(int row, int column, const QModelIndex& parent) const override;
            virtual QModelIndex parent(const QModelIndex&) const override;
            virtual QModelIndex sibling(int row, int column, const QModelIndex&) const override;
            virtual int         rowCount(const QModelIndex& parent = {}) const override;
            virtual int         columnCount(const QModelIndex& parent = {}) const override;
         #pragma endregion
         #pragma region Node data
            virtual QVariant      data(const QModelIndex&, int role) const override;
            virtual Qt::ItemFlags flags(const QModelIndex&) const override;
            #pragma region Write-access
               virtual bool setData(const QModelIndex& index, const QVariant& value, int role) override;
            #pragma endregion
         #pragma endregion
         virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
      #pragma endregion

      constexpr const ui::types::conditions::context& conditionContext() const noexcept {
         return this->_context;
      }

      QModelIndex createObjective();
      void deleteObjective(const QModelIndex&);

      QModelIndex createTarget(const QModelIndex& objective);
      void deleteTarget(const QModelIndex& target);

      condition_list targetConditions(const QModelIndex&) const;
      void setTargetConditions(const QModelIndex&, condition_list&&);

   protected:
      ui::types::conditions::context _context;
      std::vector<std::unique_ptr<Objective>> _data;
      QPointer<const QuestAliasesModel> _aliases_model;

      decltype(_data)::iterator _insertion_point_for_objective_index(uint32_t);
      void _re_sort_objective(size_t index);

      void _on_form_deleted(const dovah::form_stub&);

      void _recache_alias_name(Target&);
      void _recache_conditions(Target&);
};