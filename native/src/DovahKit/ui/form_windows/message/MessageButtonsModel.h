#pragma once
#include <cstdint>
#include "ui/models/DKGenericListModel.h"
#include "ui/types/conditions/condition.h"

namespace dovah {
   class form_stub;
}

struct MessageButtonsModelNode {
   QString text;
   std::vector<ui::types::conditions::condition> conditions;
};

class MessageButtonsModel : public DKGenericListModel<MessageButtonsModel, MessageButtonsModelNode> {
   public:
      struct Column {
         Column() = delete;
         enum enumeration : size_t {
            Name,

            __COUNT
         };
      };
      static constexpr const size_t column_count = Column::__COUNT; // override

   public:
      MessageButtonsModel(QObject* parent);

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
