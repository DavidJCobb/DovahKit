#pragma once
#include <optional>
#include "dovah/data/dialogue/emotion.h"
#include "ui/models/DKGenericListModel.h"

struct TopicInfoResponseTableviewModelNode {
   QString response_text;
   struct {
      dovah::dialogue::emotion type = dovah::dialogue::emotion::neutral;
      int32_t value = 0;
   } emotion;
   bool edited = false;
};

class TopicInfoResponseTableviewModel : public DKGenericListModel<TopicInfoResponseTableviewModel, TopicInfoResponseTableviewModelNode> {
   public:
      struct Column {
         Column() = delete;
         enum enumeration : size_t {
            Text,
            Emotion,
            Edited,

            __COUNT
         };
      };
      static constexpr const size_t column_count = Column::__COUNT; // override

   public:
      TopicInfoResponseTableviewModel(QObject* parent);

      using DKGenericListModel::clear;
      using DKGenericListModel::moveItem;
      using DKGenericListModel::moveItems;
      using DKGenericListModel::deleteItems;

      #pragma region Overrides
         QVariant data_of(const node_type& node, Qt::ItemDataRole role, size_t column) const;
         Qt::ItemFlags flags_of(const node_type&, size_t column) const;

         virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
      #pragma endregion

      std::optional<size_t> create();
      void overwrite(size_t row, const node_type& src);

      void overwriteAllItems(const std::vector<node_type>& src);
};