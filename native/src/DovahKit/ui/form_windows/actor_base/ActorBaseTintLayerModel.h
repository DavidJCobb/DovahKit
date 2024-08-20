#pragma once
#include <cstdint>
#include <vector>
#include <QAbstractItemModel>
#include <QColor>
#include <QString>

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
      
   public:
      enum class TintLayerType {
         none,
         lip_color,
         cheek_color_upper,
         eyeliner,
         eyeshadow_upper,
         eyeshadow_lower,
         skin_tone,
         facepaint,
         laugh_lines,
         cheek_color_lower,
         nose,
         chin,
         neck,
         forehead,
         dirt,
         unknown_16,
      };

      struct TintLayerPreset {
         int16_t           index = 0;
         dovah::form_stub* color = nullptr;
         float             alpha = 1;
      };

      struct TintLayer {
         uint16_t      index = 0;
         TintLayerType type = TintLayerType::none;
         QString       texture;

         dovah::form_stub* default_color = nullptr;
         std::vector<TintLayerPreset> presets;
      };

      struct TintLayerActorState {
         uint16_t preset_index = 0; // which preset to use
         std::optional<QColor> custom_color; // a custom color to use
         uint32_t alpha = 0; // fixed-point: alpha * 100
      };

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
      void importLayerUsage(const dovah::loaded_forms::ActorBase&);

      void exportLayerUsage(dovah::loaded_forms::ActorBase& dst);
};