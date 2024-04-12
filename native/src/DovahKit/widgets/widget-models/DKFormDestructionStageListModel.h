#pragma once
#include "ui/models/DKGenericListModel.h"
#include "../DKFormDestructionDataButton.h"

class DKFormDestructionStageListModel : public DKGenericListModel<DKFormDestructionStageListModel, DKFormDestructionDataButton::DestructionStage> {
   public:
      struct Column {
         Column() = delete;
         enum enumeration : size_t {
            HealthPercentage,
            SelfDPS,
            FlagCapDamage,
            FlagDisable,
            FlagDestroy,
            FlagIgnoreExternal,
            ModelDamageStage,
            Explosion,
            Debris,
            ReplacementModel,
         };
      };
      static constexpr const size_t column_count = 10; // override

   public:
      DKFormDestructionStageListModel(QObject* parent) : DKGenericListModel(parent) {}

      using DKGenericListModel::clear;
      using DKGenericListModel::deleteItems;

      #pragma region Overrides
         QVariant data_of(const node_type& node, Qt::ItemDataRole role, size_t column) const;
         Qt::ItemFlags flags_of(const node_type&, size_t column) const;

         virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
      #pragma endregion
         
      QModelIndex insertStage(const node_type&);
      void replaceStages(const std::vector<node_type>& items);
      QModelIndex setStage(int row, const node_type& src); // returns index of overwritten stage after any re-sorting, etc., that may take place

      const node_type* stage(int row) const;

   protected:
      decltype(_nodes)::iterator _insertion_point_for(unsigned int health_percentage, unsigned int damage_stage);

      static bool _sort_nodes(const node_type* a, const node_type* b);
};
