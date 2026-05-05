#pragma once
#include <cstdint>
#include <memory>
#include <optional>
#include <vector>
#include <QAbstractItemModel>
#include "ui/types/conditions/condition.h"
namespace dovah::loaded_forms {
   class Quest;
}

class QuestStagesModel : public QAbstractItemModel {
   Q_OBJECT;
   public:
      using condition_list   = std::vector<ui::types::conditions::condition>;
      using loaded_form_data = dovah::loaded_forms::Quest;

      static constexpr const size_t StageColumnCount = 1;

      struct LogEntryColumn {
         LogEntryColumn() = delete;
         enum {
            Text,
            Conditions,

            __COUNT
         };
      };
      static constexpr const size_t LogEntryColumnCount = LogEntryColumn::__COUNT;

      struct StageData {
         uint16_t id                 = 0;
         bool     keep_instance_data = false;
         bool     shutdown           = false;
         bool     startup            = false;
      };
      struct LogEntryData {
         bool complete_quest = false;
         bool fail_quest     = false;
         struct {
            QString scriptname;
            QString function;
         } fragment;
         dovah::form_stub* next_quest = nullptr;
         QString           text;
         condition_list    conditions;
      };

      // Use as the root index of a `QuestStageLogEntriesModel` when no stage is selected.
      QModelIndex noStageQMI() const noexcept;

   protected:
      struct LogEntryNode : public LogEntryData {
         struct {
            QString conditions;
         } cached;
      };
      struct StageNode : public StageData {
         std::vector<std::unique_ptr<LogEntryNode>> log_entries;
      };

   protected:
      #pragma region QMI<->pointer utils
         static bool _is_no_stage_qmi(const QModelIndex&);
         static bool _is_stage_qmi(const QModelIndex&);
         static bool _is_log_entry_qmi(const QModelIndex&);

         const StageNode* _qmi_to_stage(const QModelIndex&) const;
         StageNode* _qmi_to_stage(const QModelIndex&);
         QModelIndex _stage_to_qmi(const StageNode&, unsigned int col = 0) const;

         const LogEntryNode* _qmi_to_log_entry(const QModelIndex&) const;
         LogEntryNode* _qmi_to_log_entry(const QModelIndex&);
         QModelIndex _log_entry_to_qmi(const StageNode&, const LogEntryNode&, unsigned int col = 0) const;

         const StageNode* _parent_stage_for_log_entry_qmi(const QModelIndex&) const;
         StageNode* _parent_stage_for_log_entry_qmi(const QModelIndex&);
      #pragma endregion

   public:
      QuestStagesModel(QObject* parent = nullptr);

      void load(const loaded_form_data&);
      void save(loaded_form_data&);

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
         #pragma endregion
         virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
      #pragma endregion

      constexpr const ui::types::conditions::context& conditionContext() const noexcept {
         return this->_context;
      }

      bool isStageIDAvailable(uint16_t) const;
      std::optional<uint16_t> getAvailableStageID() const;

      QModelIndex createStage();
      void deleteStage(const QModelIndex&);

      QModelIndex createLogEntry(const QModelIndex& objective);
      void deleteLogEntry(const QModelIndex& target);

      const StageData* stage(const QModelIndex&) const;
      void setStage(const QModelIndex&, const StageData&);
      const LogEntryData* logEntry(const QModelIndex&) const;
      void setLogEntry(const QModelIndex&, LogEntryData&&);
      void setLogEntry(const QModelIndex&, const LogEntryData&);

   protected:
      ui::types::conditions::context _context;
      std::vector<std::unique_ptr<StageNode>> _stages;

      decltype(_stages)::iterator _insertion_point_for_stage_index(uint16_t);
      void _re_sort_stage(size_t index);

      void _on_form_deleted(const dovah::form_stub&);

      void _recache_conditions(LogEntryNode&);
};