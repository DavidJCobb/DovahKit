#pragma once
#include <cstdint>
#include "ui/models/DKGenericListModel.h"

namespace dovah {
   class form_stub;
}

struct ActorBaseFactionsModelNode {
   dovah::form_stub* faction = nullptr;
   int32_t rank = 0;

   struct {
      QString editorID;
   } cached;
};

class ActorBaseFactionsModel : public DKGenericListModel<ActorBaseFactionsModel, ActorBaseFactionsModelNode> {
   public:
      struct Column {
         Column() = delete;
         enum enumeration : size_t {
            Faction,
            Rank,

            __COUNT
         };
      };
      static constexpr const size_t column_count = Column::__COUNT; // override

   public:
      ActorBaseFactionsModel(QObject* parent);

      using DKGenericListModel::clear;
      using DKGenericListModel::deleteItems;

      #pragma region Overrides
         QVariant data_of(const node_type& node, Qt::ItemDataRole role, size_t column) const;
         Qt::ItemFlags flags_of(const node_type&, size_t column) const;

         virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
      #pragma endregion

      QModelIndex create();
      QModelIndex overwrite(int row, const node_type& src);
      const node_type* item(int row) const;

      void overwriteAllItems(const std::vector<node_type>& src);

      inline const bool containsFaction(const dovah::form_stub* stub) const noexcept {
         for (auto* node : this->_nodes)
            if (node->faction == stub)
               return true;
         return false;
      }
};
