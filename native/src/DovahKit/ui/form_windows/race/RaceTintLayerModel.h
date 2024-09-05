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

      struct LayerColumn {
         LayerColumn() = delete;
         enum type {
            TexturePath,
            NumPresets,
            Type,

            __COUNT
         };
      };
      static constexpr const size_t LayerColumnCount = LayerColumn::__COUNT;

      struct PresetColumn {
         PresetColumn() = delete;
         enum type {
            ColorPreview,
            Name,
            Alpha,

            __COUNT,
         };
      };
      static constexpr const size_t PresetColumnCount = PresetColumn::__COUNT;
      
   public:
      using Layer  = ui::types::face_tints::layer;
      using Preset = ui::types::face_tints::preset;

      struct LayerData {
         dovah::face_tint_type type = dovah::face_tint_type::none;
         std::string           texture;
         dovah::form_stub*     default_color = nullptr;
      };
      struct PresetData {
         float alpha = 1.0;
         struct {
            dovah::form_stub* form = nullptr;
            QColor cached;
         } color;
      };

   protected:
      static LayerData _data_of(const Layer&);
      static PresetData _data_of(const Preset&);

      static void _overwrite(Layer&, const LayerData&);
      static void _overwrite(Preset&, const PresetData&);

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
      Layer*  _layer_from_qmi(const QModelIndex&) const;
      Preset* _preset_from_qmi(const QModelIndex&) const;
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
      Layer*  _layer_by_index(dovah::face_tint_index_type) const;
      Preset* _preset_by_index(dovah::face_tint_index_type preset) const;
      Preset* _preset_by_indices(dovah::face_tint_index_type layer, dovah::face_tint_index_type preset) const;

      dovah::face_tint_index_type _allocate_new_index();
      void _try_free_index(dovah::face_tint_index_type);
      bool _index_is_in_use(dovah::face_tint_index_type) const;

   public:
      QModelIndex noPresetQMI() const; // index of no preset, for use as a model root

      void importLayers(const dovah::loaded_forms::Race&, dovah::sex);
      void exportLayers(dovah::loaded_forms::Race& dst, dovah::sex);

      #pragma region Functions for editing layers
         std::optional<dovah::face_tint_index_type> create_layer(); // returns new layer's index
         
         [[nodiscard]] std::optional<LayerData> get_layer(const QModelIndex&) const;
         [[nodiscard]] std::optional<LayerData> get_layer(dovah::face_tint_index_type) const;
         
         void overwrite_layer(const QModelIndex&, const LayerData&);
         void overwrite_layer(dovah::face_tint_index_type, const LayerData&);
         
         void remove_layer(const QModelIndex&);
         void remove_layer(dovah::face_tint_index_type);
         
         [[nodiscard]] std::optional<dovah::face_tint_index_type> layer_index(const QModelIndex&) const;
         [[nodiscard]] QModelIndex layer_qmi(dovah::face_tint_index_type) const;

         bool layer_has_color(const QModelIndex&, const dovah::form_stub&) const;
      #pragma endregion
      #pragma region Functions for editing presets
         std::optional<dovah::face_tint_index_type> create_preset(const QModelIndex& layer_qmi); // returns new preset's index
         std::optional<dovah::face_tint_index_type> create_preset(dovah::face_tint_index_type layer_qmi); // returns new preset's index
         
         [[nodiscard]] std::optional<PresetData> get_preset(const QModelIndex&);
         [[nodiscard]] std::optional<PresetData> get_preset(dovah::face_tint_index_type);
         
         void overwrite_preset(const QModelIndex&, const PresetData&);
         void overwrite_preset(dovah::face_tint_index_type, const PresetData&);
         
         void remove_preset(const QModelIndex&);
         void remove_preset(dovah::face_tint_index_type);
         
         [[nodiscard]] std::optional<dovah::face_tint_index_type> preset_index(const QModelIndex&) const;
         [[nodiscard]] QModelIndex preset_qmi(dovah::face_tint_index_type) const;
      #pragma endregion
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