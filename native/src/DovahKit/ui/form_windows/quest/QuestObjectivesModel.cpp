#include "./QuestObjectivesModel.h"
#include <optional>
#include <QStringBuilder>
#include "helpers/vectors/re_sort_item_within.h"
#include "dovah/forms/Quest.h"
#include "editor/core.h"
#include "editor/helpers/stringify_conditions.h"
#include "editor/subsystems/game_localized_strings/core.h"
#include "./QuestAliasesModel.h"

QModelIndex QuestObjectivesModel::noObjectiveQMI() const noexcept {
   return this->createIndex(0, 0, -1);
}

static std::optional<Qt::ItemDataRole> _role_for_objective_column(int c) {
   switch (c) {
      case QuestObjectivesModel::Column::Index:
         return QuestObjectivesModel::IndexRole;
      case QuestObjectivesModel::Column::IsOr:
         return QuestObjectivesModel::IsOrRole;
      case QuestObjectivesModel::Column::Text:
         return QuestObjectivesModel::TextRole;
   }
   return {};
}

#pragma region QMI<->pointer utils
   /*static*/ bool QuestObjectivesModel::_is_no_objective_qmi(const QModelIndex& qmi) {
      return ((int)qmi.internalId() == -1);
   }
   /*static*/ bool QuestObjectivesModel::_is_objective_qmi(const QModelIndex& qmi) {
      if (!qmi.isValid() || _is_no_objective_qmi(qmi))
         return false;
      return qmi.internalPointer() == nullptr;
   }
   /*static*/ bool QuestObjectivesModel::_is_target_qmi(const QModelIndex& qmi) {
      if (!qmi.isValid() || _is_no_objective_qmi(qmi))
         return false;
      return qmi.internalPointer() != nullptr;
   }

   const QuestObjectivesModel::Objective* QuestObjectivesModel::_qmi_to_objective(const QModelIndex& qmi) const {
      if (!qmi.isValid() || _is_no_objective_qmi(qmi))
         return nullptr;
      if (qmi.internalPointer()) // pointer is a parent objective; ergo QMI is a target
         return nullptr;
      auto& list = this->_data;
      auto  i    = qmi.row();
      if (i >= list.size())
         return nullptr;
      return list[i].get();
   }
   QuestObjectivesModel::Objective* QuestObjectivesModel::_qmi_to_objective(const QModelIndex& qmi) {
      return const_cast<Objective*>(std::as_const(*this)._qmi_to_objective(qmi));
   }
   QModelIndex QuestObjectivesModel::_objective_to_qmi(const Objective& obj, unsigned int col) const {
      const auto& list = this->_data;
      for (size_t i = 0; i < list.size(); ++i) {
         if (list[i].get() == &obj)
            return this->createIndex(i, col, nullptr);
      }
      return {};
   }

   const QuestObjectivesModel::Target* QuestObjectivesModel::_qmi_to_target(const QModelIndex& qmi) const {
      if (!qmi.isValid() || _is_no_objective_qmi(qmi))
         return nullptr;
      auto* objective = _parent_objective_for_target_qmi(qmi);
      if (!objective)
         return nullptr;
      auto& list = objective->targets;
      auto  i    = qmi.row();
      if (i >= list.size())
         return nullptr;
      return list[i].get();
   }
   QuestObjectivesModel::Target* QuestObjectivesModel::_qmi_to_target(const QModelIndex& qmi) {
      return const_cast<Target*>(std::as_const(*this)._qmi_to_target(qmi));
   }
   QModelIndex QuestObjectivesModel::_target_to_qmi(const Objective& obj, const Target& tgt, unsigned int col) const {
      const auto& list = obj.targets;
      for (size_t i = 0; i < list.size(); ++i) {
         if (list[i].get() == &tgt)
            return this->createIndex(i, col, (void*)& obj);
      }
      return {};
   }

   const QuestObjectivesModel::Objective* QuestObjectivesModel::_parent_objective_for_target_qmi(const QModelIndex& qmi) const {
      if (!qmi.isValid() || _is_no_objective_qmi(qmi))
         return nullptr;
      return (const Objective*)qmi.internalPointer();
   }
   QuestObjectivesModel::Objective* QuestObjectivesModel::_parent_objective_for_target_qmi(const QModelIndex& qmi) {
      return const_cast<Objective*>(std::as_const(*this)._parent_objective_for_target_qmi(qmi));
   }
#pragma endregion

QuestObjectivesModel::QuestObjectivesModel(QObject* parent) : QAbstractItemModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub) { this->_on_form_deleted(*stub); });
}

void QuestObjectivesModel::load(const loaded_form_data& src_form) {
   this->beginResetModel();
   this->_data.clear();
   this->_context = ui::types::conditions::context(src_form.stub, true);
   {
      auto& gls = dovahkit::subsystems::game_localized_strings::core::get();
      this->_data.reserve(src_form.objectives.size());
      for (auto& src_objective : src_form.objectives) {
         auto& dst_objective_ptr = this->_data.emplace_back(std::make_unique<Objective>());
         auto& dst_objective     = *dst_objective_ptr;
         dst_objective.index        = src_objective.index;
         dst_objective.or_with_prev = (bool)(src_objective.flags & loaded_form_data::Objective::flag::or_with_previous);
         dst_objective.text = gls.convert_localized_string(src_objective.text);
         for (auto& src_target : src_objective.targets) {
            auto& dst_target_ptr = dst_objective.targets.emplace_back(std::make_unique<Target>());
            auto& dst_target     = *dst_target_ptr;
            dst_target.alias_id = src_target.aliasID;
            dst_target.compass_markers_ignore_locks = (bool)(src_target.flags & loaded_form_data::Target::flag::marker_pathing_ignores_locks);
            for (auto& src_condition : src_target.conditions)
               dst_target.conditions.push_back(src_condition);

            this->_recache_alias_name(dst_target);
            this->_recache_conditions(dst_target);
         }
      }
   }
   this->endResetModel();
}
void QuestObjectivesModel::save(loaded_form_data& dst_form) {
   auto _overwrite_list = [&dst_form](const auto& src_list, auto& dst_list, auto&& overwrite) {
      size_t size = src_list.size();
      size_t i    = 0;
      if (dst_list.size() < size)
         dst_list.resize(size);
      for (; i < size; ++i) {
         const auto& src_item = *src_list[i];
         auto&       dst_item = dst_list[i];
         overwrite(src_item, dst_item);
      }
      for (; i < dst_list.size(); ++i)
         dst_list[i].clear(dst_form);
      dst_list.resize(size);
   };

   auto& gls = dovahkit::subsystems::game_localized_strings::core::get();
   _overwrite_list(
      this->_data,
      dst_form.objectives,
      [&dst_form, &gls, &_overwrite_list](const Objective& src_objective, loaded_form_data::Objective& dst_objective) {
         dst_objective.index = src_objective.index;
         dst_objective.flags = 0;
         if (src_objective.or_with_prev)
            dst_objective.flags |= loaded_form_data::Objective::flag::or_with_previous;
         gls.assign_localized_string(dst_objective.text, src_objective.text);
         _overwrite_list(
            src_objective.targets,
            dst_objective.targets,
            [&dst_form](const Target& src_target, loaded_form_data::Target& dst_target) {
               dst_target.aliasID = src_target.alias_id;
               dst_target.flags = 0;
               if (src_target.compass_markers_ignore_locks)
                  dst_target.flags |= loaded_form_data::Target::flag::marker_pathing_ignores_locks;
               dst_target.conditions.clear(dst_form);
               dst_target.conditions.append_all_of(dst_form, src_target.conditions);
            }
         );
      }
   );
}
void QuestObjectivesModel::setAliasesModel(const QuestAliasesModel* model) {
   if (this->_aliases_model) {
      QObject::disconnect(this->_aliases_model, nullptr, this, nullptr);
   }
   this->_aliases_model = model;
   if (model) {
      for (size_t i = 0; i < this->_data.size(); ++i) {
         auto& objective = *this->_data[i].get();
         for (size_t j = 0; j < objective.targets.size(); ++j) {
            auto& target   = *objective.targets[i].get();
            auto  alias_id = target.alias_id;
            if (auto* alias = model->aliasByID(target.alias_id)) {
               target.cached.alias_name = QString::fromStdString(alias->name);
            } else {
               target.cached.alias_name.clear();
            }
            auto qmi = this->createIndex(j, TargetColumn::AliasName, (void*)&objective);
            emit dataChanged(qmi, qmi);
         }
      }
      QObject::connect(model, &QuestAliasesModel::onAliasChanged, this, [this](const QModelIndex& alias_qmi) {
         auto* alias = this->_aliases_model->alias(alias_qmi);
         if (!alias)
            return;
         for (size_t i = 0; i < this->_data.size(); ++i) {
            auto& objective = *this->_data[i].get();
            for (size_t j = 0; j < objective.targets.size(); ++j) {
               auto& target = *objective.targets[i].get();
               if (target.alias_id == alias->id) {
                  target.cached.alias_name = QString::fromStdString(alias->name);
                  auto qmi = this->createIndex(j, TargetColumn::AliasName, (void*)&objective);
                  emit dataChanged(qmi, qmi);
               }
            }
         }
      });
   }
}

#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex QuestObjectivesModel::index(int row, int column, const QModelIndex& parent) const /*override*/ {
         if (row < 0 || column < 0 || _is_no_objective_qmi(parent))
            return {};
         auto& objectives = this->_data;
         if (!parent.isValid()) {
            if (column >= ColumnCount)
               return {};
            if (row >= objectives.size())
               return {};
            return this->createIndex(row, column, nullptr);
         }

         if (_is_target_qmi(parent)) // targets cannot have children
            return {};

         if (column >= TargetColumnCount)
            return {};
         auto* objective = _qmi_to_objective(parent);
         if (!objective || row >= objective->targets.size())
            return {};
         return this->createIndex(row, column, (void*)objective);
      }
      /*virtual*/ QModelIndex QuestObjectivesModel::parent(const QModelIndex& qmi) const /*override*/ {
         if (!qmi.isValid() || _is_no_objective_qmi(qmi))
            return {};
         auto* objective = _parent_objective_for_target_qmi(qmi);
         if (!objective)
            return {};
         for (size_t i = 0; i < this->_data.size(); ++i) {
            if (this->_data[i].get() == objective) {
               return this->createIndex(i, 0, nullptr);
            }
         }
         return {};
      }
      /*virtual*/ QModelIndex QuestObjectivesModel::sibling(int row, int column, const QModelIndex& qmi) const /*override*/ {
         if (row < 0 || column < 0)
            return {};
         if (!qmi.isValid())
            return {};
         if (_is_target_qmi(qmi)) {
            auto* objective = _parent_objective_for_target_qmi(qmi);
            if (!objective)
               return {};
            if (column >= TargetColumnCount)
               return {};
            if (row >= objective->targets.size())
               return {};
            return this->createIndex(row, column, (void*)objective);
         }

         if (column >= ColumnCount)
            return {};
         auto& objectives = this->_data;
         if (row >= objectives.size())
            return {};
         return this->createIndex(row, column, nullptr);
      }
      /*virtual*/ int QuestObjectivesModel::rowCount(const QModelIndex& parent) const /*override*/ {
         if (!parent.isValid())
            return this->_data.size();
         if (_is_no_objective_qmi(parent))
            return 0;
         auto* objective = _qmi_to_objective(parent);
         if (!objective)
            return 0;
         return objective->targets.size();
      }
      /*virtual*/ int QuestObjectivesModel::columnCount(const QModelIndex& parent) const /*override*/ {
         if (!parent.isValid())
            return ColumnCount;
         if (_is_objective_qmi(parent))
            return TargetColumnCount;
         return 0;
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant QuestObjectivesModel::data(const QModelIndex& qmi, int role) const /*override*/ {
         if (auto* objective = _qmi_to_objective(qmi)) {
            if (role == Qt::EditRole) {
               auto role_opt = _role_for_objective_column(qmi.column());
               if (!role_opt.has_value())
                  return false;
               role = role_opt.value();
            }
            switch (role) {
               case IndexRole:
                  return objective->index;
               case IsOrRole:
                  return objective->or_with_prev;
               case TextRole:
                  return objective->text;

               case Qt::DisplayRole:
               case Qt::ToolTipRole:
                  switch (qmi.column()) {
                     case Column::Index:
                        return objective->index;
                     case Column::Text:
                        return objective->text;
                  }
                  break;

               case Qt::CheckStateRole:
                  if (qmi.column() == Column::IsOr) {
                     if (objective->or_with_prev)
                        return Qt::CheckState::Checked;
                     return Qt::CheckState::Unchecked;
                  }
                  break;
            }
            return {};
         }

         if (auto* target = _qmi_to_target(qmi)) {
            switch (role) {
               case TargetAliasIDRole:
                  return target->alias_id;
               case TargetIgnoreLocksRole:
                  return target->compass_markers_ignore_locks;

               case Qt::DisplayRole:
               case Qt::ToolTipRole:
                  switch (qmi.column()) {
                     case TargetColumn::AliasName:
                        return target->cached.alias_name;
                     case TargetColumn::Conditions:
                        return target->cached.conditions;
                  }
                  break;
            }
            return {};
         }

         return {};
      }
      /*virtual*/ Qt::ItemFlags QuestObjectivesModel::flags(const QModelIndex& qmi) const /*override*/ {
         if (!qmi.isValid())
            return {};
         auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled;
         if (_is_target_qmi(qmi))
            flags |= Qt::ItemFlag::ItemNeverHasChildren;
         return flags;
      }
      #pragma region Write-access
         /*virtual*/ bool QuestObjectivesModel::setData(const QModelIndex& qmi, const QVariant& value, int role) /*override*/ {
            if (!qmi.isValid())
               return false;

            if (auto* objective = _qmi_to_objective(qmi)) {
               std::optional<int> changed_column;
               bool re_sort_item = false;
               if (role == Qt::EditRole) {
                  auto role_opt = _role_for_objective_column(qmi.column());
                  if (!role_opt.has_value())
                     return false;
                  role = role_opt.value();
               }
               switch (role) {
                  case IndexRole:
                     if (!value.canConvert<int>())
                        return false;
                     objective->index = value.toInt();
                     changed_column = Column::Index;
                     re_sort_item   = true;
                     break;
                  case IsOrRole:
                     if (!value.canConvert<bool>())
                        return false;
                     objective->or_with_prev = value.toBool();
                     changed_column = Column::IsOr;
                     break;
                  case TextRole:
                     if (!value.canConvert<QString>())
                        return false;
                     objective->text = value.toString();
                     changed_column = Column::Text;
                     break;
                  default:
                     return false;
               }
               if (changed_column.has_value()) {
                  auto changed = qmi.siblingAtColumn(changed_column.value());
                  emit dataChanged(changed, changed);
               }
               if (re_sort_item)
                  this->_re_sort_objective(qmi.row());
               return true;
            }

            if (auto* target = _qmi_to_target(qmi)) {
               switch (role) {
                  case TargetAliasIDRole:
                     if (!value.canConvert<int32_t>())
                        return false;
                     {
                        auto v = value.toInt();
                        if (target->alias_id == v)
                           return true;
                        target->alias_id = v;
                        this->_recache_alias_name(*target);

                        auto changed = qmi.siblingAtColumn(TargetColumn::AliasName);
                        emit dataChanged(changed, changed);
                     }
                     break;
                  case TargetIgnoreLocksRole:
                     if (!value.canConvert<bool>())
                        return false;
                     target->compass_markers_ignore_locks = value.toBool();
                     break;
                  default:
                     return false;
               }
               return true;
            }

            return false;
         }
      #pragma endregion
   #pragma endregion
   /*virtual*/ QVariant QuestObjectivesModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
      if (orientation != Qt::Orientation::Horizontal)
         return {};
      switch (role) {
         case Qt::DisplayRole:
         case Qt::ToolTipRole:
            break;
         default:
            return {};
      }
      switch (section) {
         case Column::Index:
            return tr("Index");
         case Column::IsOr:
            return tr("OR");
         case Column::Text:
            return tr("Display Text");
      }
      return {};
   }
#pragma endregion

QModelIndex QuestObjectivesModel::createObjective() {
   constexpr const auto default_index = 0;

   auto&  list = this->_data;
   auto   it   = _insertion_point_for_objective_index(default_index);
   size_t i    = std::distance(list.begin(), it);

   auto item_ptr = std::make_unique<Objective>();
   item_ptr->index = default_index;

   this->beginInsertRows({}, i, i);
   list.insert(it, std::move(item_ptr));
   this->endInsertRows();

   return this->createIndex(i, 0, nullptr);
}
void QuestObjectivesModel::deleteObjective(const QModelIndex& qmi) {
   if (!_is_objective_qmi(qmi))
      return;
   auto& list = this->_data;
   int   i    = qmi.row();
   if (i >= list.size())
      return;
   this->beginRemoveRows({}, i, i);
   list.erase(list.begin() + i);
   this->endRemoveRows();
}

QModelIndex QuestObjectivesModel::createTarget(const QModelIndex& objective_qmi) {
   auto* objective = _qmi_to_objective(objective_qmi);
   if (!objective)
      return {};
   auto&  list = objective->targets;
   size_t i    = list.size();
   list.reserve(i + 1);
   auto item_ptr = std::make_unique<Target>();
   this->beginInsertRows(objective_qmi, i, i);
   list.emplace_back(std::move(item_ptr));
   this->endInsertRows();
   return this->createIndex(i, 0, (void*)objective);
}
void QuestObjectivesModel::deleteTarget(const QModelIndex& target_qmi) {
   auto* target = _qmi_to_target(target_qmi);
   if (!target)
      return;
   auto* objective = _parent_objective_for_target_qmi(target_qmi);
   auto  i         = target_qmi.row();
   if (i >= objective->targets.size())
      return;
   this->beginRemoveRows(_objective_to_qmi(*objective), i, i);
   objective->targets.erase(objective->targets.begin() + i);
   this->endRemoveRows();
}

QuestObjectivesModel::condition_list QuestObjectivesModel::targetConditions(const QModelIndex& qmi) const {
   if (auto* target = _qmi_to_target(qmi)) {
      return target->conditions;
   }
   return {};
}
void QuestObjectivesModel::setTargetConditions(const QModelIndex& qmi, condition_list&& list) {
   auto* target = _qmi_to_target(qmi);
   if (!target)
      return;
   target->conditions = std::move(list);
   this->_recache_conditions(*target);
   auto c_qmi = qmi.siblingAtColumn(TargetColumn::Conditions);
   emit dataChanged(c_qmi, c_qmi);
}

decltype(QuestObjectivesModel::_data)::iterator QuestObjectivesModel::_insertion_point_for_objective_index(uint32_t index) {
   return std::upper_bound(
      this->_data.begin(),
      this->_data.end(),
      index,
      [](uint32_t index, const auto& obj_ptr) -> bool {
         return index < obj_ptr->index;
      }
   );
}
void QuestObjectivesModel::_re_sort_objective(size_t from) {
   auto& list = this->_data;
   if (from >= list.size())
      return;
   bool moved = false;
   cobb::vectors::re_sort_item_within(
      list,
      list.begin() + from,
      [](const auto& a_ptr, const auto& b_ptr) -> bool {
         return a_ptr->index < b_ptr->index;
      },
      [&moved, this, &list](decltype(_data)::iterator from_it, decltype(_data)::iterator to_it) {
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

void QuestObjectivesModel::_on_form_deleted(const dovah::form_stub& stub) {
   for (size_t i = 0; i < this->_data.size(); ++i) {
      auto& objective = *this->_data[i];
      for (size_t j = 0; j < objective.targets.size(); ++j) {
         auto& target  = *objective.targets[j];
         bool  changed = false;
         for (auto& cnd : target.conditions) {
            if (cnd.sever_outbound_references_to(&stub))
               changed = true;
         }
         if (changed) {
            this->_recache_conditions(target);
            auto qmi = this->createIndex(j, TargetColumn::Conditions, (void*)&objective);
            emit dataChanged(qmi, qmi);
         }
      }
   }
}

void QuestObjectivesModel::_recache_alias_name(Target& target) {
   if (this->_aliases_model) {
      auto* alias = this->_aliases_model->aliasByID(target.alias_id);
      if (alias)
         target.cached.alias_name = QString::fromStdString(alias->name);
   }
}
void QuestObjectivesModel::_recache_conditions(Target& target) {
   auto&  str  = target.cached.conditions;
   auto&  list = target.conditions;
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