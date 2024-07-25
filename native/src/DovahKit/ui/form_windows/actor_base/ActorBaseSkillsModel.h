#pragma once
#include <array>
#include <cstdint>
#include <optional>
#include <QAbstractItemModel>
#include "dovah/data/skills.h"

namespace dovah {
   class form_stub;
}

class ActorBaseSkillsModel final : public QAbstractItemModel {
   Q_OBJECT;
   public:
      ActorBaseSkillsModel(QObject* parent);
      
      #pragma region QAbstractItemModel overrides
         #pragma region Hierarchy
            virtual QModelIndex index(int row, int column, const QModelIndex& parent) const override;
            virtual QModelIndex parent(const QModelIndex& index) const override;
            virtual QModelIndex sibling(int row, int column, const QModelIndex& index) const override;
            virtual int         rowCount(const QModelIndex& parent = {}) const override;
            virtual int         columnCount(const QModelIndex& parent = {}) const override;
         #pragma endregion
         #pragma region Node data
            virtual QVariant      data(const QModelIndex& index, int role) const override;
            virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
         #pragma endregion
         virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
      #pragma endregion

      void setClass(dovah::form_stub*);
      void setLevel(uint32_t);
      void setRace(dovah::form_stub*);
      void setSkillOffset(dovah::skill, uint8_t);
      void setOffsetsUsed(bool);

      constexpr uint8_t offsetOf(dovah::skill s) const noexcept {
         if ((size_t)s >= this->_state.skills.size())
            return 0;
         return this->_state.skills[(size_t)s].offset;
      }

   protected:
      struct SkillInfo {
         uintmax_t computed = 0;
         uint8_t   offset = 0;
      };
      struct SkillBonus {
         dovah::skill skill;
         uint8_t      bonus = 0;
      };

      struct {
         dovah::form_stub* class_form    = nullptr;
         uint32_t          level         = 0;
         bool              using_offsets = false;

         std::array<std::optional<SkillBonus>, 7>  racial_bonuses;
         std::array<SkillInfo, dovah::skill_count> skills;
      } _state;

      void _recalcStats();
};
