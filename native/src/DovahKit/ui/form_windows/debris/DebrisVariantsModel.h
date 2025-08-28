#pragma once
#include <cstdint>
#include "ui/models/DKGenericListModel.h"
#include "ui/types/nif_for_form.h"

struct DebrisVariantsModelNode {
   ui::types::nif_for_form path;
   uint8_t chance = 0;
   bool    has_collision = false;
};

class DebrisVariantsModel : public DKGenericListModel<DebrisVariantsModel, DebrisVariantsModelNode> {
   public:
      struct Column {
         Column() = delete;
         enum enumeration : size_t {
            Path,
            Chance,
            HasCollision,

            __COUNT
         };
      };
      static constexpr const size_t column_count = Column::__COUNT; // override

   public:
      DebrisVariantsModel(QObject* parent);

      using DKGenericListModel::clear;
      using DKGenericListModel::deleteItems;

      #pragma region Overrides
         QVariant data_of(const node_type& node, Qt::ItemDataRole role, size_t column) const;
         Qt::ItemFlags flags_of(const node_type&, size_t column) const;
         constexpr bool allow_inbound_drag_and_drop() const {
            return true;
         }

         virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
      #pragma endregion

      QModelIndex create();
      QModelIndex overwrite(int row, const node_type& src);
      const node_type* item(int row) const;

      void overwriteAllItems(const std::vector<node_type>& src);
};
