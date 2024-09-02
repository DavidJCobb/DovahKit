#pragma once
#include <cstdint>
#include <optional>
#include <vector>
#include <QAbstractItemModel>
#include <QColor>
#include <QString>
#include "dovah/data/face_tints.h"
#include "dovah/utils/data_by_sex.h"

namespace dovah {
   namespace loaded_forms {
      class ActorBase;
      class Race;
   }
   class form_stub;
}

class ActorBaseTintLayerModel : public QAbstractItemModel {
   Q_OBJECT;
   public:
      ActorBaseTintLayerModel(QObject* parent = nullptr);
      ~ActorBaseTintLayerModel();

      struct Column {
         Column() = delete;
         enum type {
            TexturePath,
            Color, // no text, but rather, shaded in with the given color
            Alpha, // "100%" etc.

            __COUNT
         };
      };
      static constexpr const size_t ColumnCount = Column::__COUNT;
      
   public:
      struct TintLayerPreset {
         dovah::face_tint_index_type index = 0;
         float alpha = 1;
         struct {
            QColor cached;
            dovah::form_stub* form = nullptr;
         } color;
      };

      struct TintLayer {
         dovah::face_tint_index_type index = 0;
         dovah::face_tint_type       type = dovah::face_tint_type::none;
         QString texture;

         dovah::form_stub* default_color = nullptr;
         std::vector<TintLayerPreset> presets;

         const TintLayerPreset* preset_by_index(uint16_t) const;
         void recache_color(dovah::form_stub&);
         void recache_all_colors();
      };

      struct TintLayerActorState {
         dovah::face_tint_index_type layer_index  = 0; // which layer to configure
         dovah::face_tint_index_type preset_index = 0; // which preset to use
         QColor   color;
         uint32_t alpha = 0; // fixed-point: alpha * 100
      };

   protected:
      struct {
         dovah::data_by_sex<std::vector<TintLayer*>> layers; // layers defined on the race
         std::vector<TintLayerActorState*> states; // settings defined on the actor

         dovah::sex sex = dovah::sex::female;
      } _data;

      void _clear_all_layers();
      void _clear_all_states();
      void _clear_silent();
      void _clear(); // emits model-reset signals

   public:
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

   public:
      void importLayerDefinitions(const dovah::loaded_forms::Race&);
      void importLayerStates(const dovah::loaded_forms::ActorBase&);
      void importLayerState(const TintLayerActorState&);
      void setSex(dovah::sex);

      void exportLayerStates(dovah::loaded_forms::ActorBase& dst);

      std::optional<uint16_t> layerIndex(const QModelIndex&) const;
      std::optional<uint16_t> layerIndex(size_t row) const;

      [[nodiscard]] std::optional<TintLayer> layerDefinitionByIndex(uint16_t layer_index) const;
      const TintLayerActorState* layerStateByIndex(uint16_t index) const;
};