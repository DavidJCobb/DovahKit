#include "./QuestStagesModel.h"
#include <optional>
#include <QStringBuilder>
#include "helpers/vectors/re_sort_item_within.h"
#include "dovah/forms/Quest.h"
#include "editor/core.h"
#include "editor/subsystems/game_localized_strings/core.h"
#include "editor/helpers/stringify_conditions.h"
#include "./QuestAliasesModel.h"

QModelIndex QuestStagesModel::noStageQMI() const noexcept {
   return this->createIndex(0, 0, -1);
}

#pragma region QMI<->pointer utils
   /*static*/ bool QuestStagesModel::_is_no_stage_qmi(const QModelIndex& qmi) {
      return ((int)qmi.internalId() == -1);
   }
   /*static*/ bool QuestStagesModel::_is_stage_qmi(const QModelIndex& qmi) {
      if (!qmi.isValid() || _is_no_stage_qmi(qmi))
         return false;
      return qmi.internalPointer() == nullptr;
   }
   /*static*/ bool QuestStagesModel::_is_log_entry_qmi(const QModelIndex& qmi) {
      if (!qmi.isValid() || _is_no_stage_qmi(qmi))
         return false;
      return qmi.internalPointer() != nullptr;
   }

   const QuestStagesModel::StageNode* QuestStagesModel::_qmi_to_stage(const QModelIndex& qmi) const {
      if (!qmi.isValid() || _is_no_stage_qmi(qmi))
         return nullptr;
      if (qmi.internalPointer()) // pointer is a parent objective; ergo QMI is a target
         return nullptr;
      auto& list = this->_stages;
      auto  i    = qmi.row();
      if (i >= list.size())
         return nullptr;
      return list[i].get();
   }
   QuestStagesModel::StageNode* QuestStagesModel::_qmi_to_stage(const QModelIndex& qmi) {
      return const_cast<StageNode*>(std::as_const(*this)._qmi_to_stage(qmi));
   }
   QModelIndex QuestStagesModel::_stage_to_qmi(const StageNode& obj, unsigned int col) const {
      const auto& list = this->_stages;
      for (size_t i = 0; i < list.size(); ++i) {
         if (list[i].get() == &obj)
            return this->createIndex(i, col, nullptr);
      }
      return {};
   }

   const QuestStagesModel::LogEntryNode* QuestStagesModel::_qmi_to_log_entry(const QModelIndex& qmi) const {
      if (!qmi.isValid() || _is_no_stage_qmi(qmi))
         return nullptr;
      auto* stage = _parent_stage_for_log_entry_qmi(qmi);
      if (!stage)
         return nullptr;
      auto& list = stage->log_entries;
      auto  i    = qmi.row();
      if (i >= list.size())
         return nullptr;
      return list[i].get();
   }
   QuestStagesModel::LogEntryNode* QuestStagesModel::_qmi_to_log_entry(const QModelIndex& qmi) {
      return const_cast<LogEntryNode*>(std::as_const(*this)._qmi_to_log_entry(qmi));
   }
   QModelIndex QuestStagesModel::_log_entry_to_qmi(const StageNode& stage, const LogEntryNode& log, unsigned int col) const {
      const auto& list = stage.log_entries;
      for (size_t i = 0; i < list.size(); ++i) {
         if (list[i].get() == &log)
            return this->createIndex(i, col, (void*)&stage);
      }
      return {};
   }

   const QuestStagesModel::StageNode* QuestStagesModel::_parent_stage_for_log_entry_qmi(const QModelIndex& qmi) const {
      if (!qmi.isValid() || _is_no_stage_qmi(qmi))
         return nullptr;
      return (const StageNode*)qmi.internalPointer();
   }
   QuestStagesModel::StageNode* QuestStagesModel::_parent_stage_for_log_entry_qmi(const QModelIndex& qmi) {
      return const_cast<StageNode*>(std::as_const(*this)._parent_stage_for_log_entry_qmi(qmi));
   }
#pragma endregion

QuestStagesModel::QuestStagesModel(QObject* parent) : QAbstractItemModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub) { this->_on_form_deleted(*stub); });
}

void QuestStagesModel::load(const loaded_form_data& src_form) {
   this->beginResetModel();
   this->_stages.clear();
   this->_context = ui::types::conditions::context(src_form.stub, true);
   {
      auto& gls = dovahkit::subsystems::game_localized_strings::core::get();
      this->_stages.reserve(src_form.stages.size());
      for (auto& src_stage : src_form.stages) {
         auto& dst_stage_ptr = this->_stages.emplace_back(std::make_unique<StageNode>());
         auto& dst_stage     = *dst_stage_ptr;
         dst_stage.id        = src_stage.index;
         dst_stage.keep_instance_data = (bool)(src_stage.flags & loaded_form_data::Stage::flag::keep_instance_data);
         dst_stage.shutdown = (bool)(src_stage.flags & loaded_form_data::Stage::flag::shutdown);
         dst_stage.startup  = (bool)(src_stage.flags & loaded_form_data::Stage::flag::startup);
         for (auto& src_log_entry : src_stage.entries) {
            auto& dst_log_entry_ptr = dst_stage.log_entries.emplace_back(std::make_unique<LogEntryNode>());
            auto& dst_log_entry     = *dst_log_entry_ptr;
            dst_log_entry.complete_quest = (bool)(src_log_entry.flags & loaded_form_data::LogEntry::flag::complete);
            dst_log_entry.fail_quest     = (bool)(src_log_entry.flags & loaded_form_data::LogEntry::flag::fail);
            dst_log_entry.next_quest     = src_log_entry.next_quest_id.get_form_stub();
            dst_log_entry.text           = gls.convert_localized_string(src_log_entry.journal_text);
            {
               auto& src_frag = src_log_entry.fragment;
               auto& dst_frag = dst_log_entry.fragment;
               dst_frag.scriptname = QString::fromStdString(src_frag.filename);
               dst_frag.function   = QString::fromStdString(src_frag.function);
            }
            for (auto& src_condition : src_log_entry.conditions)
               dst_log_entry.conditions.push_back(src_condition);

            this->_recache_conditions(dst_log_entry);
         }
      }
   }
   this->endResetModel();
}
void QuestStagesModel::save(loaded_form_data& dst_form) {
   auto _overwrite_list = [&dst_form](const auto& src_list, auto& dst_list, auto&& overwrite) {
      size_t size = src_list.size();
      size_t i    = 0;
      if (dst_list.size() < size)
         dst_list.resize(size);
      for (; i < size; ++i) {
         const auto& src_item = *src_list[i];
         auto&       dst_item = dst_list[i];
         if constexpr (std::is_invocable_v<decltype(overwrite), decltype(src_item), decltype(dst_item), size_t>) {
            overwrite(src_item, dst_item, i);
         } else {
            overwrite(src_item, dst_item);
         }
      }
      for (; i < dst_list.size(); ++i)
         dst_list[i].clear(dst_form);
      dst_list.resize(size);
   };

   auto& gls = dovahkit::subsystems::game_localized_strings::core::get();
   _overwrite_list(
      this->_stages,
      dst_form.stages,
      [&dst_form, &gls, &_overwrite_list](const StageNode& src_stage, loaded_form_data::Stage& dst_stage, size_t si) {
      dst_stage.index = src_stage.id;
         dst_stage.flags = 0;
         dst_stage.index = src_stage.id;
         if (src_stage.keep_instance_data)
            dst_stage.flags |= loaded_form_data::Stage::flag::keep_instance_data;
         if (src_stage.shutdown)
            dst_stage.flags |= loaded_form_data::Stage::flag::shutdown;
         if (src_stage.startup)
            dst_stage.flags |= loaded_form_data::Stage::flag::startup;
         _overwrite_list(
            src_stage.log_entries,
            dst_stage.entries,
            [&dst_form, &gls, si](const LogEntryNode& src_log_entry, loaded_form_data::LogEntry& dst_log_entry, size_t li) {
               gls.assign_localized_string(dst_log_entry.journal_text, src_log_entry.text);
               dst_log_entry.flags = 0;
               if (src_log_entry.complete_quest)
                  dst_log_entry.flags |= loaded_form_data::LogEntry::flag::complete;
               if (src_log_entry.fail_quest)
                  dst_log_entry.flags |= loaded_form_data::LogEntry::flag::fail;
               dst_log_entry.next_quest_id.set(dst_form, src_log_entry.next_quest);
               {
                  auto& src_frag = src_log_entry.fragment;
                  auto& dst_frag = dst_log_entry.fragment;
                  if (!src_frag.scriptname.isEmpty() && !src_frag.function.isEmpty()) {
                     dst_frag.stage_id    = si;
                     dst_frag.entry_index = li;
                     dst_frag.unknown08   = 0x01;
                     dst_frag.filename    = src_frag.scriptname.toStdString();
                     dst_frag.function    = src_frag.function.toStdString();
                  } else {
                     dst_frag = {};
                  }
               }
               dst_log_entry.conditions.clear(dst_form);
               dst_log_entry.conditions.append_all_of(dst_form, src_log_entry.conditions);
            }
         );
      }
   );
}

#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex QuestStagesModel::index(int row, int column, const QModelIndex& parent) const /*override*/ {
         if (row < 0 || column < 0 || _is_no_stage_qmi(parent))
            return {};
         auto& objectives = this->_stages;
         if (!parent.isValid()) {
            if (column >= StageColumnCount)
               return {};
            if (row >= objectives.size())
               return {};
            return this->createIndex(row, column, nullptr);
         }

         if (_is_log_entry_qmi(parent)) // log entries cannot have children
            return {};

         if (column >= LogEntryColumnCount)
            return {};
         auto* stage = _qmi_to_stage(parent);
         if (!stage || row >= stage->log_entries.size())
            return {};
         return this->createIndex(row, column, (void*)stage);
      }
      /*virtual*/ QModelIndex QuestStagesModel::parent(const QModelIndex& qmi) const /*override*/ {
         if (!qmi.isValid() || _is_no_stage_qmi(qmi))
            return {};
         auto* stage = _parent_stage_for_log_entry_qmi(qmi);
         if (!stage)
            return {};
         return _stage_to_qmi(*stage);
      }
      /*virtual*/ QModelIndex QuestStagesModel::sibling(int row, int column, const QModelIndex& qmi) const /*override*/ {
         if (row < 0 || column < 0)
            return {};
         if (!qmi.isValid())
            return {};
         if (_is_log_entry_qmi(qmi)) {
            auto* stage = _parent_stage_for_log_entry_qmi(qmi);
            if (!stage)
               return {};
            if (column >= LogEntryColumnCount)
               return {};
            if (row >= stage->log_entries.size())
               return {};
            return this->createIndex(row, column, (void*)stage);
         }

         if (column >= StageColumnCount)
            return {};
         auto& stage = this->_stages;
         if (row >= stage.size())
            return {};
         return this->createIndex(row, column, nullptr);
      }
      /*virtual*/ int QuestStagesModel::rowCount(const QModelIndex& parent) const /*override*/ {
         if (!parent.isValid())
            return this->_stages.size();
         if (_is_no_stage_qmi(parent))
            return 0;
         auto* stage = _qmi_to_stage(parent);
         if (!stage)
            return 0;
         return stage->log_entries.size();
      }
      /*virtual*/ int QuestStagesModel::columnCount(const QModelIndex& parent) const /*override*/ {
         if (!parent.isValid())
            return StageColumnCount;
         if (_is_stage_qmi(parent))
            return LogEntryColumnCount;
         return 0;
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant QuestStagesModel::data(const QModelIndex& qmi, int role) const /*override*/ {
         if (auto* stage = _qmi_to_stage(qmi)) {
            switch (role) {
               case Qt::DisplayRole:
               case Qt::ToolTipRole:
                  switch (qmi.column()) {
                     case 0:
                        return stage->id;
                  }
                  break;
            }
            return {};
         }

         if (auto* log_entry = _qmi_to_log_entry(qmi)) {
            switch (role) {
               case Qt::DisplayRole:
               case Qt::ToolTipRole:
                  switch (qmi.column()) {
                     case LogEntryColumn::Text:
                        return log_entry->text;
                     case LogEntryColumn::Conditions:
                        return log_entry->cached.conditions;
                  }
                  break;
            }
            return {};
         }

         return {};
      }
      /*virtual*/ Qt::ItemFlags QuestStagesModel::flags(const QModelIndex& qmi) const /*override*/ {
         if (!qmi.isValid())
            return {};
         auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled;
         if (_is_log_entry_qmi(qmi))
            flags |= Qt::ItemFlag::ItemNeverHasChildren;
         return flags;
      }
   #pragma endregion
   /*virtual*/ QVariant QuestStagesModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
      if (orientation != Qt::Orientation::Horizontal)
         return {};
      switch (role) {
         case Qt::DisplayRole:
         case Qt::ToolTipRole:
            break;
         default:
            return {};
      }
      if (section == 0)
         return tr("Stage ID");
      return {};
   }
#pragma endregion

bool QuestStagesModel::isStageIDAvailable(uint16_t id) const {
   for (const auto& stage_ptr : this->_stages)
      if (stage_ptr->id == id)
         return false;
   return true;
}
std::optional<uint16_t> QuestStagesModel::getAvailableStageID() const {
   if (this->_stages.empty())
      return 0;
   const auto* stage = this->_stages.back().get();
   if (stage->id < std::numeric_limits<uint16_t>::max())
      return stage->id + 1;
   if (this->_stages.size() == 1)
      return stage->id - 1;

   for (size_t i = 1; i < this->_stages.size(); ++i) {
      auto* prev = this->_stages[i - 1].get();
      auto* here = this->_stages[i].get();
      if (here->id > prev->id + 1)
         return prev->id + 1;
   }

   return {};
}

QModelIndex QuestStagesModel::createStage() {
   auto id = this->getAvailableStageID();
   if (!id.has_value())
      return {};

   auto&  list = this->_stages;
   auto   it   = _insertion_point_for_stage_index(id.value());
   size_t i    = std::distance(list.begin(), it);

   auto item_ptr = std::make_unique<StageNode>();
   item_ptr->id = id.value();

   this->beginInsertRows({}, i, i);
   list.insert(it, std::move(item_ptr));
   this->endInsertRows();

   return this->createIndex(i, 0, nullptr);
}
void QuestStagesModel::deleteStage(const QModelIndex& qmi) {
   if (!_is_stage_qmi(qmi))
      return;
   auto& list = this->_stages;
   int   i    = qmi.row();
   if (i >= list.size())
      return;
   this->beginRemoveRows({}, i, i);
   list.erase(list.begin() + i);
   this->endRemoveRows();
}

QModelIndex QuestStagesModel::createLogEntry(const QModelIndex& stage_qmi) {
   auto* stage = _qmi_to_stage(stage_qmi);
   if (!stage)
      return {};
   auto&  list = stage->log_entries;
   size_t i    = list.size();
   list.reserve(i + 1);
   auto item_ptr = std::make_unique<LogEntryNode>();
   this->beginInsertRows(stage_qmi, i, i);
   list.emplace_back(std::move(item_ptr));
   this->endInsertRows();
   return this->createIndex(i, 0, (void*)stage);
}
void QuestStagesModel::deleteLogEntry(const QModelIndex& target_qmi) {
   auto* target = _qmi_to_log_entry(target_qmi);
   if (!target)
      return;
   auto* stage = _parent_stage_for_log_entry_qmi(target_qmi);
   auto  i     = target_qmi.row();
   if (i >= stage->log_entries.size())
      return;
   this->beginRemoveRows(_stage_to_qmi(*stage), i, i);
   stage->log_entries.erase(stage->log_entries.begin() + i);
   this->endRemoveRows();
}

const QuestStagesModel::StageData* QuestStagesModel::stage(const QModelIndex& qmi) const {
   const auto* s = _qmi_to_stage(qmi);
   if (s)
      return s;
   return nullptr;
}
void QuestStagesModel::setStage(const QModelIndex& qmi, const StageData& v) {
   auto* s = _qmi_to_stage(qmi);
   if (!s)
      return;
   auto prior_id    = s->id;
   bool keep_old_id = false;
   if (v.id != prior_id && !this->isStageIDAvailable(v.id))
      keep_old_id = true;

   s->StageData::operator=(v); // ensure we don't clobber derived-class fields
   if (keep_old_id)
      s->id = prior_id;
   else if (s->id != prior_id) {
      auto c = qmi.siblingAtColumn(0);
      emit dataChanged(c, c);
      this->_re_sort_stage(qmi.row());
   }
}
const QuestStagesModel::LogEntryData* QuestStagesModel::logEntry(const QModelIndex& qmi) const {
   const auto* s = _qmi_to_log_entry(qmi);
   if (s)
      return s;
   return nullptr;
}
void QuestStagesModel::setLogEntry(const QModelIndex& qmi, LogEntryData&& v) {
   auto* l = _qmi_to_log_entry(qmi);
   if (!l)
      return;

   l->LogEntryData::operator=(std::move(v));
   this->_recache_conditions(*l);

   auto tl = qmi.siblingAtColumn(0);
   auto br = qmi.siblingAtColumn(LogEntryColumnCount - 1);
   emit dataChanged(tl, br);
}
void QuestStagesModel::setLogEntry(const QModelIndex& qmi, const LogEntryData& v) {
   auto* l = _qmi_to_log_entry(qmi);
   if (!l)
      return;

   l->LogEntryData::operator=(v);
   this->_recache_conditions(*l);

   auto tl = qmi.siblingAtColumn(0);
   auto br = qmi.siblingAtColumn(LogEntryColumnCount - 1);
   emit dataChanged(tl, br);
}

decltype(QuestStagesModel::_stages)::iterator QuestStagesModel::_insertion_point_for_stage_index(uint16_t index) {
   return std::upper_bound(
      this->_stages.begin(),
      this->_stages.end(),
      index,
      [](uint16_t index, const auto& obj_ptr) -> bool {
         return index < obj_ptr->id;
      }
   );
}
void QuestStagesModel::_re_sort_stage(size_t from) {
   auto& list = this->_stages;
   if (from >= list.size())
      return;
   bool moved = false;
   cobb::vectors::re_sort_item_within(
      list,
      list.begin() + from,
      [](const auto& a_ptr, const auto& b_ptr) -> bool {
         return a_ptr->id < b_ptr->id;
      },
      [&moved, this, &list](decltype(_stages)::iterator from_it, decltype(_stages)::iterator to_it) {
         moved = true;
         size_t from  = std::distance(list.begin(), from_it);
         size_t to    = std::distance(list.begin(), to_it);
         this->beginMoveRows(
            {},
            from, // first to move
            from, // last  to move
            {},
            (to < from) ? to : to + 1 // Qt API design jank
         );
      }
   );
   if (moved)
      this->endMoveRows();
}

void QuestStagesModel::_on_form_deleted(const dovah::form_stub& stub) {
   for (size_t i = 0; i < this->_stages.size(); ++i) {
      auto& stage = *this->_stages[i];
      for (size_t j = 0; j < stage.log_entries.size(); ++j) {
         auto& log_entry          = *stage.log_entries[j];
         bool  changed            = false;
         bool  changed_conditions = false;
         for (auto& cnd : log_entry.conditions) {
            if (cnd.sever_outbound_references_to(&stub)) {
               changed            = true;
               changed_conditions = true;
            }
         }
         if (log_entry.next_quest == &stub) {
            log_entry.next_quest = nullptr;
            changed = true;
         }
         if (changed) {
            if (changed_conditions) {
               this->_recache_conditions(log_entry);
               auto qmi = this->createIndex(j, LogEntryColumn::Conditions, (void*)&stage);
               emit dataChanged(qmi, qmi);
            }
         }
      }
   }
}

void QuestStagesModel::_recache_conditions(LogEntryNode& log_entry) {
   auto&  str  = log_entry.cached.conditions;
   auto&  list = log_entry.conditions;
   size_t size = list.size();
   if (size > 0) {
      for (size_t i = 0; i < size - 1; ++i) {
         const auto& cnd = list[i];
         str += editor_helpers::stringify_condition(cnd, this->_context);
         str += " " % editor_helpers::stringify_condition_boolean_operator(cnd) % " ";
      }
      str += editor_helpers::stringify_condition(list[size - 1], this->_context);
   }
}