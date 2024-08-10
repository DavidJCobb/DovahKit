#pragma once
#include <array>
#include <cstdint>
#include <optional>
#include <QAbstractItemModel>
#include "dovah/data/skills.h"
#include "dovah/utils/compute_classed_stat_points.h"

namespace dovah {
   class form_stub;
}

class ActorBaseSkillsModel final : public QAbstractItemModel {
   Q_OBJECT;
   public:
      using skill_value_type  = uint8_t; // should match ActorBase
      using skill_value_array = std::array<skill_value_type, dovah::skill_count>;

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

      void setAllData(const skill_value_array& skill_offsets, const skill_value_array& computed_skills);
      void setOffsetsUsed(bool);
      std::optional<dovah::skill> skillAtRow(int) const;

   protected:
      struct SkillInfo {
         uint8_t computed = 0;
         uint8_t offset   = 0;
      };

      struct {
         bool using_offsets = false;
         std::array<SkillInfo, dovah::skill_count> skills;
      } _state;
};
