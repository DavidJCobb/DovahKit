#pragma once
#include <cstdint>
#include "ui/models/DKGenericListModel.h"
#include "dovah/forms/Weather.h"

namespace dovah {
   class form_stub;
}

struct WeatherSoundsModelNode {
   using weather_sound_type = dovah::loaded_forms::Weather::weather_sound_type;

   dovah::form_stub*  sound      = nullptr;
   weather_sound_type sound_type = weather_sound_type::default_;
   struct {
      QString sound_editor_id;
   } cached;
};

class WeatherSoundsModel : public DKGenericListModel<WeatherSoundsModel, WeatherSoundsModelNode> {
   public:
      struct Column {
         Column() = delete;
         enum enumeration : size_t {
            Sound,
            Type,

            __COUNT
         };
      };
      static constexpr const size_t column_count = Column::__COUNT; // override
      
      using weather_sound_type = node_type::weather_sound_type;

   public:
      WeatherSoundsModel(QObject* parent);

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
};
