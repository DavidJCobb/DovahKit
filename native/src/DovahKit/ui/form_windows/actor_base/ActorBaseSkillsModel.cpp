#include "./ActorBaseSkillsModel.h"
#include <cmath> // round
#include "dovah/forms/Class.h"
#include "dovah/form_stub.h"
#include "editor/helpers/skill_name_to_string.h"
#include "editor/subsystems/game_settings/core.h"
#include "editor/core.h"
#include "helpers/string/strlen.h"

#include "dovah/forms/Class.h"
#include "dovah/forms/Race.h"

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
      
void ActorBaseSkillsModel::setAllData(const skill_value_array& skill_offsets, const skill_value_array& computed_skills) {
   for (size_t i = 0; i < dovah::skill_count; ++i) {
      auto& dst   = this->_state.skills[i];
      auto  value = (unsigned int)skill_offsets[i] + (unsigned int)computed_skills[i];
      //
      dst.offset   = skill_offsets[i];
      dst.computed = std::min<unsigned int>(value, std::numeric_limits<skill_value_type>::max());
   }
   
   int col = 1;
   if (this->_state.using_offsets)
      col = 0;
   auto tl = this->index(0, col, {});
   auto br = this->index(dovah::skill_count - 1, col, {});
   emit dataChanged(tl, br);
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
}
std::optional<dovah::skill> ActorBaseSkillsModel::skillAtRow(int r) const {
   if (r < 0 || r >= this->_state.skills.size())
      return {};
   return (dovah::skill)r;
}