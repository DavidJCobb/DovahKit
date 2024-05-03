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
         
      QModelIndex appendStage(const node_type&);
      void replaceStages(const std::vector<node_type>& items);
      void setStage(int row, const node_type& src);

      const node_type* stage(int row) const;
};
