#pragma once
#include <cstdint>
#include "ui/models/DKGenericListModel.h"

namespace dovah {
   class form_stub;
}

struct ActorBasePerksModelNode {
   dovah::form_stub* perk = nullptr;
   int32_t rank = 0;

   struct {
      QString editorID;
   } cached;
};

class ActorBasePerksModel : public DKGenericListModel<ActorBasePerksModel, ActorBasePerksModelNode> {
   public:
      struct Column {
         Column() = delete;
         enum enumeration : size_t {
            Perk,
            Rank,

            __COUNT
         };
      };
      static constexpr const size_t column_count = Column::__COUNT; // override

   public:
      ActorBasePerksModel(QObject* parent);

      using DKGenericListModel::clear;
      using DKGenericListModel::deleteItems;

      #pragma region Overrides
         QVariant data_of(const node_type& node, Qt::ItemDataRole role, size_t column) const;
         Qt::ItemFlags flags_of(const node_type&, size_t column) const;
         constexpr bool allow_inbound_drag_and_drop() const {
            return true;
         }

         virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
         
         #pragma region Drag-and-drop
            virtual bool canDropMimeData(const QMimeData*, Qt::DropAction, int row, int column, const QModelIndex& parent) const override;
            virtual bool dropMimeData(const QMimeData*, Qt::DropAction, int row, int column, const QModelIndex& parent) override;
            virtual QStringList mimeTypes() const override;
            virtual Qt::DropActions supportedDropActions() const override;
         #pragma endregion
      #pragma endregion

      QModelIndex create();
      QModelIndex overwrite(int row, const node_type& src);
      const node_type* item(int row) const;

      void overwriteAllItems(const std::vector<node_type>& src);

      inline const bool containsPerk(const dovah::form_stub* stub) const noexcept {
         for (auto* node : this->_nodes)
            if (node->perk == stub)
               return true;
         return false;
      }
};
