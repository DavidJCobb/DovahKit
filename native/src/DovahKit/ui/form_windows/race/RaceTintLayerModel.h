#pragma once
#include <optional>
#include <vector>
#include <QAbstractItemModel>
#include <QIdentityProxyModel>
#include <QColor>
#include <QString>
#include "dovah/data/sex.h"
#include "ui/types/face_tints/layer.h"
#include "ui/types/face_tints/preset.h"

namespace dovah {
   namespace loaded_forms {
      class Race;
   }
   class form_stub;
}

//
// Model for the tint layers defined on a single race and a single sex.
//
class RaceTintLayerModel : public QAbstractItemModel {
   Q_OBJECT;
   public:
      RaceTintLayerModel(QObject* parent = nullptr);
      ~RaceTintLayerModel();

      struct Column {
         Column() = delete;
         enum type {
            TexturePath,
            NumPresets,
            Type,

            __COUNT
         };
      };
      static constexpr const size_t ColumnCount = Column::__COUNT;

      struct PresetColumn {
         PresetColumn() = delete;
         enum type {
            ColorPreview,
            Name,
            Alpha,

            __COUNT,
         };
      };
      static constexpr const size_t PresetColumnCount = Column::__COUNT;
      
   public:
      using Layer       = ui::types::face_tints::layer;
      using LayerPreset = ui::types::face_tints::preset;
      struct LayerData {
         dovah::face_tint_type type = dovah::face_tint_type::none;
         std::string           texture;
         dovah::form_stub*     default_color = nullptr;
      };

   protected:

      // Track all used indices on the Race.
      // If `this_index` == `layer_index`, then `this_index` is the index of a layer.
      // Else, `this_index` is the index of a preset, and `layer_index` the index of its containing layer.
      struct _index {
         dovah::face_tint_index_type this_index;
         dovah::face_tint_index_type layer_index;
      };

   protected:
      struct {
         std::vector<Layer*> layers; // layers defined on the race
         std::vector<bool>   indices_used;
      } _data;
      struct {
         dovah::face_tint_index_type start_from = 1;
      } _tint_index_info;

      void _clear_all_layers();
      void _clear_silent();
      void _clear(); // emits model-reset signals

      bool _qmi_is_none(const QModelIndex&) const; // see `noneIndex`
      bool _qmi_is_layer(const QModelIndex&) const;
      bool _qmi_is_preset(const QModelIndex&) const;
      //
      Layer* _layer_from_qmi(const QModelIndex&) const;
      LayerPreset* _preset_from_qmi(const QModelIndex&) const;
      //
      QModelIndex _qmi_of_preset(Layer*, size_t preset_row, size_t col = 0) const;
      QModelIndex _qmi_of_layer(size_t row, size_t col = 0) const;

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

   protected:
      std::optional<size_t> _row_for_layer(dovah::face_tint_index_type) const;
      Layer* _layer_by_index(dovah::face_tint_index_type) const;
      LayerPreset* _preset_by_index(dovah::face_tint_index_type preset) const;
      LayerPreset* _preset_by_indices(dovah::face_tint_index_type layer, dovah::face_tint_index_type preset) const;

      dovah::face_tint_index_type _allocate_new_index();
      void _try_free_index(dovah::face_tint_index_type);
      bool _index_is_in_use(dovah::face_tint_index_type) const;

   public:
      QModelIndex noneIndex() const; // index of no preset, for use as a model root

      void importLayers(const dovah::loaded_forms::Race&, dovah::sex);
      void exportLayers(dovah::loaded_forms::Race& dst, dovah::sex);

      std::optional<dovah::face_tint_index_type> addLayer(); // returns new layer's index
      void removeLayerByIndex(dovah::face_tint_index_type);
      void removeLayerByRow(size_t);
      //
      std::optional<LayerData> getLayerData(const QModelIndex&) const;
      std::optional<LayerData> getLayerData(dovah::face_tint_index_type) const;
      void setLayerData(const QModelIndex&, const LayerData&);
      void setLayerData(dovah::face_tint_index_type, const LayerData&);

      [[nodiscard]] std::optional<LayerPreset> getLayerPresetByIndex(dovah::face_tint_index_type preset);
      [[nodiscard]] std::optional<LayerPreset> getLayerPresetByIndex(dovah::face_tint_index_type layer, dovah::face_tint_index_type preset);
      [[nodiscard]] std::optional<LayerPreset> getLayerPresetByRow(dovah::face_tint_index_type layer, size_t row);
      std::optional<dovah::face_tint_index_type> addLayerPreset(dovah::face_tint_index_type layer); // returns new preset's index
      void replaceLayerPreset(const LayerPreset&); // uses the passed-in preset's index to identify the destination; fails silently if no match
      void replaceLayerPreset(dovah::face_tint_index_type layer, const LayerPreset&); // uses the passed-in preset's index to identify the destination; fails silently if no match
      void removeLayerPresetByIndex(dovah::face_tint_index_type layer, dovah::face_tint_index_type preset);
      void removeLayerPresetByRow(dovah::face_tint_index_type layer, size_t row);

      std::optional<dovah::face_tint_index_type> layerIndex(const QModelIndex&) const;
      std::optional<dovah::face_tint_index_type> layerIndex(size_t row) const;

      [[nodiscard]] std::optional<Layer> layerDefinitionByIndex(dovah::face_tint_index_type layer_index) const;
};

//
// Qt's model/view system supports all of the following:
// 
//  - Pointing multiple views at the same model
// 
//  - List, table, and tree models
// 
//  - Hierarchical models with varying node types (these types are abstracted behind the model 
//    index and item data system)
// 
//  - Choosing the root model node used by a given view
// 
//      - Therefore, having two views point at different nodes on the same model
// 
//      - Therefore, having two views point at differently-structured sets of data 
//        that exist within the same hierarchy
// 
// In that light, it feels like a bit of an oversight that it doesn't support the following:
// 
//  - Returning a varying arrangement of headers depending on a given root node (i.e. by 
//    having `headerData` take a QModelIndex parameter).
// 
// Fortunately, it's relatively trivial to set that up with a proxy model.
//
class RaceTintLayerPresetsModel : public QIdentityProxyModel {
   public:
      using Column = RaceTintLayerModel::PresetColumn;
      static constexpr const size_t ColumnCount = RaceTintLayerModel::PresetColumnCount;

   public:
      using QIdentityProxyModel::QIdentityProxyModel;

      virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
};