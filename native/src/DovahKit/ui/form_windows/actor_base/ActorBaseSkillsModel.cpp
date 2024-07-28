#include "./ActorBaseSkillsModel.h"
#include <cmath> // round
#include "dovah/forms/Class.h"
#include "dovah/form_stub.h"
#include "editor/helpers/skill_name_to_string.h"
#include "editor/core.h"

#include "dovah/forms/Class.h"
#include "dovah/forms/Race.h"

namespace {
   static int _get_iAVDskillsLevelUp() {
      {
         auto& editor = DovahKitCore::get();

         dovah::loaded_game_setting data;
         if (editor.get_loaded_game_setting("iAVDSkillsLevelUp", data))
            return data.value.i;
      }
      for (auto& dfn : dovah::game_settings)
         if (_strnicmp("iAVDSkillsLevelUp", dfn.name, sizeof("iAVDSkillsLevelUp")))
            return dfn.default_value.i;
      return 8;
   }
}

ActorBaseSkillsModel::ActorBaseSkillsModel(QObject* parent) : QAbstractItemModel(parent) {
}
      
#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex ActorBaseSkillsModel::index(int row, int col, const QModelIndex& parent) const /*override*/ {
         if (row < 0 || row >= this->_state.skills.size())
            return {};
         if (col < 0 || col >= this->columnCount())
            return {};
         if (parent.isValid())
            return {};
         return this->createIndex(row, col, nullptr);
      }
      /*virtual*/ QModelIndex ActorBaseSkillsModel::parent(const QModelIndex& index) const /*override*/ {
         return {};
      }
      /*virtual*/ QModelIndex ActorBaseSkillsModel::sibling(int row, int column, const QModelIndex& index) const /*override*/ {
         if (!index.isValid())
            return {};
         return this->index(row, column, {});
      }
      /*virtual*/ int ActorBaseSkillsModel::rowCount(const QModelIndex& parent) const /*override*/ {
         return this->_state.skills.size();
      }
      /*virtual*/ int ActorBaseSkillsModel::columnCount(const QModelIndex& parent) const /*override*/ {
         if (this->_state.using_offsets)
            return 3;
         return 2;
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant ActorBaseSkillsModel::data(const QModelIndex& index, int role) const /*override*/ {
         if (!index.isValid())
            return {};
         if (index.row() >= this->_state.skills.size())
            return {};
         auto& src = this->_state.skills[index.row()];

         enum class _column {
            offset,
            value,
            skill,
         };
         _column col;
         {
            auto i = index.column();
            if (this->_state.using_offsets) {
               constexpr const auto columns = std::array{
                  _column::offset,
                  _column::value,
                  _column::skill,
               };
               if (i >= columns.size())
                  return {};
               col = columns[i];
            } else {
               constexpr const auto columns = std::array{
                  _column::value,
                  _column::skill,
               };
               if (i >= columns.size())
                  return {};
               col = columns[i];
            }
         }
         switch (role) {
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
               switch (col) {
                  case _column::offset:
                     return src.offset;
                  case _column::value:
                     return src.computed;
                  case _column::skill:
                     return editor_helpers::skill_name_to_string((dovah::skill)index.row());
               }
               break;
            case Qt::TextAlignmentRole:
               switch (col) {
                  case _column::offset:
                  case _column::value:
                     return (int)(Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
               }
               break;

         }
         return {};
      }
      /*virtual*/ Qt::ItemFlags ActorBaseSkillsModel::flags(const QModelIndex& index) const /*override*/ {
         auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
         return flags;
      }
   #pragma endregion
      /*virtual*/ QVariant ActorBaseSkillsModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
         if (orientation != Qt::Orientation::Horizontal)
            return {};
         if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
            return {};
         if (this->_state.using_offsets) {
            switch (section) {
               case 0:
                  return tr("Offset");
               case 1:
                  return tr("Total");
               case 2:
                  return tr("Skill Name");
            }
         } else {
            switch (section) {
               case 0:
                  return tr("Value");
               case 1:
                  return tr("Skill Name");
            }
         }
         return {};
      }
#pragma endregion

void ActorBaseSkillsModel::setClass(dovah::form_stub* stub) {
   if (stub == this->_state.class_form)
      return;
   this->_state.class_form = stub;
   this->_recalcStats();
}
void ActorBaseSkillsModel::setLevel(uint32_t level) {
   if (this->_state.level == level)
      return;
   this->_state.level = level;
   this->_recalcStats();
}
void ActorBaseSkillsModel::setRace(dovah::form_stub* stub) {
   if (stub && stub->form_type != dovah::form_type::race)
      return;
   this->_state.racial_bonuses = {};
   if (stub) {
      auto loaded = stub->load().ptr_cast<dovah::loaded_forms::Race>();
      if (loaded) {
         static_assert(
            std::tuple_size_v<decltype(decltype(dovah::loaded_forms::Race::stats)::skill_boosts)> == std::tuple_size_v<decltype(this->_state.racial_bonuses)>,
            "Ensure we have room for all of the racial bonuses in a Race form!"
         );

         for(size_t i = 0; i < loaded->stats.skill_boosts.size(); ++i) {
            auto& src = loaded->stats.skill_boosts[i];
            auto& dst = this->_state.racial_bonuses[i];
            if (!src.has_value()) {
               dst = {};
               continue;
            }
            dst.emplace();
            dst.value().skill = src.value().skill;
            dst.value().bonus = src.value().boost;
         }
      }
   }
   this->_recalcStats();
}
void ActorBaseSkillsModel::setSkillOffset(dovah::skill skill, uint8_t offset) {
   if ((size_t)skill >= this->_state.skills.size())
      return;
   auto& dst = this->_state.skills[(size_t)skill];
   if (dst.offset == offset)
      return;
   dst.offset = offset;
   if (this->_state.using_offsets) {
      auto qmi = this->index((size_t)skill, 0, {});
      emit dataChanged(qmi, qmi);
   }
}
void ActorBaseSkillsModel::setOffsetsUsed(bool v) {
   if (this->_state.using_offsets == v)
      return;
   this->_state.using_offsets = v;
   if (v) {
      this->beginInsertColumns({}, 0, 0);
      this->endInsertColumns();
   } else {
      this->beginRemoveColumns({}, 0, 0);
      this->endRemoveColumns();
   }
   this->_recalcStats();
}

void ActorBaseSkillsModel::_recalcStats() {
   int iAVDskillsLevelUp = _get_iAVDskillsLevelUp();

   auto& dst_list = this->_state.skills;

   // Gather racial bonuses.
   for (size_t i = 0; i < dst_list.size(); ++i) {
      auto& dst = dst_list[i];
      dst.computed = 0;

      for (auto& item : this->_state.racial_bonuses) {
         if (!item.has_value())
            continue;
         if (item.value().skill == (dovah::skill)i)
            dst.computed += item.value().bonus;
      }
   }
   if (this->_state.using_offsets) {
      for (auto& dst : dst_list)
         dst.computed += dst.offset;
   }

   int  skill_points = this->_state.level * iAVDskillsLevelUp;
   bool used_class   = false;
   if (this->_state.class_form) {
      auto loaded = this->_state.class_form->load().ptr_cast<dovah::loaded_forms::Class>();
      if (loaded) {
         used_class = true;

         size_t total_weight = 0;
         for (auto w : loaded->skill_weights)
            total_weight += w;

         for (size_t i = 0; i < dovah::skill_count; ++i) {
            auto  weight = loaded->skill_weights[i];
            auto& dst    = dst_list[i];

            float v = (float)weight / total_weight;
            v *= skill_points;

            dst.computed += round(v);
         }
      }
   }
   if (!used_class) {
      int portion   = skill_points / dovah::skill_count;
      int remaining = skill_points;
      for (auto& dst : dst_list) {
         if (portion > remaining) {
            dst.computed += remaining;
         } else {
            dst.computed += portion;
            remaining    -= portion;
         }
      }
   }

   int col = 1;
   if (this->_state.using_offsets)
      col = 0;
   auto tl = this->index(0, col, {});
   auto br = this->index(dovah::skill_count - 1, col, {});
   emit dataChanged(tl, br);
}