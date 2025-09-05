#pragma once
#include <cstdint>
#include "dovah/forms/LensFlare.h"
#include "ui/models/DKGenericListModel.h"

struct LensFlareSpritesModelNode : dovah::loaded_forms::LensFlare::sprite {
   struct {
      QString id;
      QString texture;
      QString size;
   } _cached;
};

class LensFlareSpritesModel : public DKGenericListModel<LensFlareSpritesModel, LensFlareSpritesModelNode> {
   public:
      struct Column {
         Column() = delete;
         enum enumeration : size_t {
            Name,
            Texture,
            Position,
            Tint,
            Opacity,
            Size,

            __COUNT
         };
      };
      static constexpr const size_t column_count = Column::__COUNT; // override

   public:
      LensFlareSpritesModel(QObject* parent);

      using DKGenericListModel::clear;
      using DKGenericListModel::deleteItems;
      using DKGenericListModel::moveItem;

      #pragma region Overrides
         QVariant data_of(const node_type& node, Qt::ItemDataRole role, size_t column) const;
         Qt::ItemFlags flags_of(const node_type&, size_t column) const;
         constexpr bool allow_inbound_drag_and_drop() const {
            return true;
         }

         virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
      #pragma endregion

      QModelIndex create();
      QModelIndex overwrite(int row, const dovah::loaded_forms::LensFlare::sprite& src);
      const dovah::loaded_forms::LensFlare::sprite* item(int row) const;

      void overwriteAllItems(const std::vector<dovah::loaded_forms::LensFlare::sprite>& src);

   protected:
      void _recache_item(node_type&);
};
