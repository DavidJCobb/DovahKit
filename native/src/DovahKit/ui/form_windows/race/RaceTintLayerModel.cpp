#include "./RaceTintLayerModel.h"
#include <QBrush>
#include "dovah/forms/Race.h"
#include "editor/core.h"

RaceTintLayerModel::RaceTintLayerModel(QObject* parent) : QAbstractItemModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, &RaceTintLayerModel::_clear);
   QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) {
      if (stub->form_type == dovah::form_type::color) {
         for (auto* layer : this->_data.layers)
            layer->recache_color(*stub);
      }
   });
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub, bool just_being_flagged) {
      if (stub->form_type == dovah::form_type::color) {
         for (auto* layer : this->_data.layers) {
            for (auto& preset : layer->presets) {
               if (preset.color.form == stub) {
                  preset.color = {}; // TODO: Can the color of a preset be nullptr, or should we remove the preset?
               }
            }
            if (layer->default_color == stub)
               layer->default_color = nullptr;
         }
      }
   });
}
RaceTintLayerModel::~RaceTintLayerModel() {
   this->_clear_silent();
}

void RaceTintLayerModel::_clear_all_layers() {
   this->_data.indices_used.clear();
   {
      auto& list = this->_data.layers;
      for (auto* layer : list)
         delete layer;
      list.clear();
   }
}
void RaceTintLayerModel::_clear_silent() {
   this->_clear_all_layers();
}
void RaceTintLayerModel::_clear() {
   this->beginResetModel();
   this->_clear_silent();
   this->endResetModel();
}

bool RaceTintLayerModel::_qmi_is_none(const QModelIndex& qmi) const {
   return qmi.internalId() == -1;
}
bool RaceTintLayerModel::_qmi_is_layer(const QModelIndex& qmi) const {
   return !_qmi_is_none(qmi) && qmi.isValid() && qmi.model() == this && qmi.internalPointer() == nullptr;
}
bool RaceTintLayerModel::_qmi_is_preset(const QModelIndex& qmi) const {
   return !_qmi_is_none(qmi) && qmi.isValid() && qmi.model() == this && qmi.internalPointer() != nullptr;
}
//
RaceTintLayerModel::Layer* RaceTintLayerModel::_layer_from_qmi(const QModelIndex& qmi) const {
   if (_qmi_is_layer(qmi)) {
      auto i = qmi.row();
      if (i >= this->_data.layers.size())
         return nullptr;
      return this->_data.layers[i];
   }
   if (_qmi_is_preset(qmi)) {
      return (Layer*)qmi.internalPointer();
   }
   return nullptr;
}
RaceTintLayerModel::LayerPreset* RaceTintLayerModel::_preset_from_qmi(const QModelIndex& qmi) const {
   if (!_qmi_is_preset(qmi))
      return nullptr;
   auto* layer = _layer_from_qmi(qmi);
   if (!layer)
      return {};
   auto i = qmi.row();
   if (i >= layer->presets.size())
      return {};
   return &layer->presets[i];
}
//
QModelIndex RaceTintLayerModel::_qmi_of_preset(Layer* layer, size_t preset_row, size_t col) const {
   if (!layer)
      return {};
   if (preset_row >= layer->presets.size())
      return {};
   return this->createIndex(preset_row, col, layer);
}
QModelIndex RaceTintLayerModel::_qmi_of_layer(size_t row, size_t col) const {
   if (row >= this->_data.layers.size())
      return {};
   if (col >= ColumnCount)
      return {};
   return this->createIndex(row, col, nullptr);
}

#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex RaceTintLayerModel::index(int row, int col, const QModelIndex& parent) const /*override*/ {
         if (row < 0 || col < 0)
            return {};
         size_t layer_count = this->_data.layers.size();
         if (parent.isValid()) {
            if (_qmi_is_preset(parent) || _qmi_is_none(parent))
               //
               // If `parent` is the index of a preset, abort.
               //
               return {};
            //
            // Generate the index of a preset.
            //
            auto* layer = _layer_from_qmi(parent);
            if (!layer)
               return {};
            return _qmi_of_preset(layer, row, col);
         } else {
            //
            // Generate the index of a layer.
            //
            return _qmi_of_layer(row, col);
         }
      }
      /*virtual*/ QModelIndex RaceTintLayerModel::parent(const QModelIndex& index) const /*override*/ {
         if (_qmi_is_preset(index)) {
            if (auto* layer = index.internalPointer()) {
               for (size_t i = 0; i < this->_data.layers.size(); ++i)
                  if (layer == this->_data.layers[i])
                     return this->createIndex(i, 0, nullptr);
            }
         }
         return {};
      }
      /*virtual*/ QModelIndex RaceTintLayerModel::sibling(int row, int column, const QModelIndex& index) const /*override*/ {
         if (row < 0 || column < 0 || !index.isValid() || _qmi_is_none(index))
            return {};
         if (_qmi_is_layer(index))
            return this->index(row, column, {});
         if (_qmi_is_preset(index)) {
            auto* layer = _layer_from_qmi(index);
            if (layer) {
               if (row < layer->presets.size() && column < PresetColumnCount)
                  return this->createIndex(row, column, layer);
            }
         }
         return {};
      }
      /*virtual*/ int RaceTintLayerModel::rowCount(const QModelIndex& parent) const /*override*/ {
         if (_qmi_is_none(parent) || _qmi_is_preset(parent))
            return 0;
         if (_qmi_is_layer(parent)) {
            auto* layer = _layer_from_qmi(parent);
            if (layer)
               return layer->presets.size();
            return 0;
         }
         return this->_data.layers.size();
      }
      /*virtual*/ int RaceTintLayerModel::columnCount(const QModelIndex& parent) const /*override*/ {
         if (_qmi_is_preset(parent) || _qmi_is_none(parent))
            return PresetColumnCount;
         return ColumnCount;
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant RaceTintLayerModel::data(const QModelIndex& index, int role) const /*override*/ {
         if (_qmi_is_none(index))
            return {};
         if (!index.isValid())
            return {};
         if (_qmi_is_layer(index)) {
            const auto* layer = _layer_from_qmi(index);
            if (!layer)
               return {};
            switch (index.column()) {
               case Column::TexturePath:
                  switch (role) {
                     case Qt::DisplayRole:
                     case Qt::ToolTipRole:
                        return QString::fromStdString(layer->texture);
                  }
                  break;
               case Column::NumPresets:
                  switch (role) {
                     case Qt::DisplayRole:
                     case Qt::ToolTipRole:
                        return layer->presets.size();
                  }
                  break;
               case Column::Type:
                  if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
                     switch (layer->type) {
                        using enum dovah::face_tint_type;
                        case none:
                           return tr("None", "tint layer type");
                        case lip_color:
                           return tr("Lip Color", "tint layer type");
                        case cheek_color_upper:
                           return tr("Cheek Color Upper", "tint layer type");
                        case eyeliner:
                           return tr("Eyeliner", "tint layer type");
                        case eyeshadow_upper:
                           return tr("Eyeshadow Upper", "tint layer type");
                        case eyeshadow_lower:
                           return tr("Eyeshadow Lower", "tint layer type");
                        case skin_tone:
                           return tr("Skin Tone", "tint layer type");
                        case facepaint:
                           return tr("Facepaint", "tint layer type");
                        case laugh_lines:
                           return tr("Laugh Lines", "tint layer type");
                        case cheek_color_lower:
                           return tr("Cheek Color Lower", "tint layer type");
                        case nose:
                           return tr("Nose", "tint layer type");
                        case chin:
                           return tr("Chin", "tint layer type");
                        case neck:
                           return tr("Neck", "tint layer type");
                        case forehead:
                           return tr("Forehead", "tint layer type");
                        case dirt:
                           return tr("Dirt", "tint layer type");
                        case unknown_16:
                           return tr("Unknown", "tint layer type");
                     }
                  }
                  break;
            }
            return {};
         }
         if (_qmi_is_preset(index)) {
            const auto* preset = _preset_from_qmi(index);
            if (!preset)
               return {};
            switch (index.column()) {
               case PresetColumn::ColorPreview:
                  if (role == Qt::BackgroundRole) {
                     return QBrush(preset->color.cached);
                  }
                  break;
               case PresetColumn::Name:
                  switch (role) {
                     case Qt::DisplayRole:
                     case Qt::ToolTipRole:
                        if (preset->color.form) {
                           return QString::fromStdString(preset->color.form->editorID);
                        } else {
                           return tr("NONE");
                        }
                        break;
                  }
                  break;
               case PresetColumn::Alpha:
                  if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
                     return QString::number(preset->alpha, 'f', 2);
                  }
                  break;
            }
            return {};
         }
         return {};
      }
      /*virtual*/ Qt::ItemFlags RaceTintLayerModel::flags(const QModelIndex& index) const /*override*/ {
         if (_qmi_is_none(index))
            return {};
         if (!index.isValid())
            return {};
         return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
      }
   #pragma endregion
   /*virtual*/ QVariant RaceTintLayerModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
      if (orientation != Qt::Orientation::Horizontal)
         return {};
      switch (role) {
         case Qt::DisplayRole:
         case Qt::ToolTipRole:
            switch (section) {
               case Column::TexturePath:
                  return tr("Texture");
               case Column::NumPresets:
                  return tr("# Presets");
               case Column::Type:
                  return tr("Type");
            }
            break;
      }
      return {};
   }
#pragma endregion

std::optional<size_t> RaceTintLayerModel::_row_for_layer(dovah::face_tint_index_type index) const {
   if (!this->_index_is_in_use(index))
      return {};
   const auto& list = this->_data.layers;
   for (size_t i = 0; i < list.size(); ++i)
      if (list[i]->index == index)
         return i;
   return {};
}
RaceTintLayerModel::Layer* RaceTintLayerModel::_layer_by_index(dovah::face_tint_index_type index) const {
   auto row = this->_row_for_layer(index);
   if (row.has_value())
      return this->_data.layers[row.value()];
   return nullptr;
}
RaceTintLayerModel::LayerPreset* RaceTintLayerModel::_preset_by_index(dovah::face_tint_index_type pi) const {
   if (!this->_index_is_in_use(pi))
      return nullptr;
   for (auto* layer : this->_data.layers)
      for (auto& preset : layer->presets)
         if (preset.index == pi)
            return &preset;
   return nullptr;
}
RaceTintLayerModel::LayerPreset* RaceTintLayerModel::_preset_by_indices(dovah::face_tint_index_type li, dovah::face_tint_index_type pi) const {
   if (!this->_index_is_in_use(pi))
      return nullptr;
   if (auto* layer = this->_layer_by_index(li)) {
      for (auto& preset : layer->presets)
         if (preset.index == pi)
            return &preset;
   }
   return nullptr;
}

dovah::face_tint_index_type RaceTintLayerModel::_allocate_new_index() {
   auto&  used = this->_data.indices_used;
   size_t i    = this->_tint_index_info.start_from;
   for (; i < used.size(); ++i) {
      if (!used[i]) {
         used[i] = true;
         return i;
      }
   }
   used.push_back(true);
   return i;
}
void RaceTintLayerModel::_try_free_index(dovah::face_tint_index_type i) {
   if (i < this->_tint_index_info.start_from)
      //
      // Indices that were already in use before we started editing are not safe to free.
      //
      return;
   auto& used = this->_data.indices_used;
   if (i >= used.size())
      return;
   used[i] = false;
}
bool RaceTintLayerModel::_index_is_in_use(dovah::face_tint_index_type i) const {
   auto& used = this->_data.indices_used;
   if (i >= used.size())
      return false;
   return used[i];
}

QModelIndex RaceTintLayerModel::noneIndex() const {
   return this->createIndex(0, 0, -1);
}

void RaceTintLayerModel::importLayers(const dovah::loaded_forms::Race& race, dovah::sex sex) {
   this->beginResetModel();
   this->_clear_all_layers();
   this->_tint_index_info.start_from = race.tint_layer_count + 1;
   {
      auto& src = race.by_sex[sex].head_data.face_tints;
      auto& dst = this->_data.layers;

      auto _set_index = [this](dovah::face_tint_index_type i) {
         auto& list = this->_data.indices_used;
         if (i >= list.size())
            list.resize(i + 1);
         list[i] = true;
      };

      for (auto& src_layer : src) {
         auto& dst_layer = dst.emplace_back();
         dst_layer = new Layer;
         dst_layer->index   = src_layer.index;
         dst_layer->texture = src_layer.texture;
         dst_layer->type    = src_layer.type;
         _set_index(src_layer.index);
         for (auto& src_preset : src_layer.presets) {
            auto& dst_preset = dst_layer->presets.emplace_back();
            dst_preset.alpha = src_preset.alpha;
            dst_preset.index = src_preset.index;
            dst_preset.color.form = src_preset.color.get_form_stub();
            _set_index(src_preset.index);
         }
         dst_layer->default_color = src_layer.default_color.get_form_stub();
         dst_layer->recache_all_colors();
      }
   }
   {
      //
      // Guard against non-unique indices (e.g. due to data being improperly edited by a 
      // user in xEdit, in Dovahscript, etc.). If an index is used multiple times, then 
      // take the entities (layers and/or presets) that reuse the index and allocate a 
      // new index for each of them.
      //
      std::vector<bool> seen;
      seen.resize(this->_data.indices_used.size());
      for (auto* layer : this->_data.layers) {
         if (seen[layer->index]) {
            //
            // Index is already in use. Renumber it.
            //
            layer->index = _allocate_new_index();
         }
         seen[layer->index] = true;
         for (auto& preset : layer->presets) {
            if (seen[preset.index]) {
               //
               // Index is already in use. Renumber it.
               //
               preset.index = _allocate_new_index();
            }
            seen[preset.index] = true;
         }
      }
   }
   this->endResetModel();
}
void RaceTintLayerModel::exportLayers(dovah::loaded_forms::Race& race, dovah::sex sex) {
   auto& dst = race.by_sex[sex].head_data.face_tints;

   {  // Clear the destination list (without screwing up use info).
      for (auto& layer : dst) {
         layer.default_color.set(race, nullptr);
         for (auto& preset : layer.presets) {
            preset.color.set(race, nullptr);
         }
      }
      dst.clear();
   }

   size_t next_tint_index = this->_tint_index_info.start_from;
   for (auto* src_item : this->_data.layers) {
      auto& dst_item = dst.emplace_back();
      dst_item.index   = src_item->index;
      dst_item.texture = src_item->texture;
      dst_item.default_color.set(race, src_item->default_color);
      for (auto& src_preset : src_item->presets) {
         auto& dst_preset = dst_item.presets.emplace_back();
         dst_preset.index = src_preset.index;
         dst_preset.alpha = src_preset.alpha;
         dst_preset.color.set(race, src_preset.color.form);
      }
   }
   if (next_tint_index > std::numeric_limits<dovah::face_tint_index_type>::max()) {
      race.tint_layer_count = std::numeric_limits<dovah::face_tint_index_type>::max();
   } else {
      race.tint_layer_count = std::min(race.tint_layer_count, (dovah::face_tint_index_type)next_tint_index);
   }
}

std::optional<dovah::face_tint_index_type> RaceTintLayerModel::addLayer() {
   auto& list = this->_data.layers;

   this->beginInsertRows({}, list.size(), list.size());
   auto  li    = this->_allocate_new_index();
   auto& ptr   = list.emplace_back();
   auto* layer = ptr = new Layer;
   layer->index = li;
   this->endInsertRows();

   return li;
}
void RaceTintLayerModel::removeLayerByIndex(dovah::face_tint_index_type li) {
   auto& list = this->_data.layers;
   for (size_t lr = 0; lr < list.size(); ++lr) {
      auto* layer = list[lr];
      if (layer->index == li) {
         this->beginRemoveRows({}, lr, lr);
         {
            for (auto& preset : layer->presets)
               this->_try_free_index(preset.index);
            this->_try_free_index(layer->index);
         }
         delete layer;
         list.erase(list.begin() + lr);
         this->endRemoveRows();
         return;
      }
   }
}
void RaceTintLayerModel::removeLayerByRow(size_t lr) {
   auto& list = this->_data.layers;
   if (lr >= list.size())
      return;
   this->beginRemoveRows({}, lr, lr);
   auto* layer = list[lr];
   {
      for (auto& preset : layer->presets)
         this->_try_free_index(preset.index);
      this->_try_free_index(layer->index);
   }
   delete layer;
   list.erase(list.begin() + lr);
   this->endRemoveRows();
}
//
std::optional<RaceTintLayerModel::LayerData> RaceTintLayerModel::getLayerData(const QModelIndex& qmi) const {
   auto* layer = _layer_from_qmi(qmi);
   if (!layer)
      return {};

   LayerData data;
   data.default_color = layer->default_color;
   data.texture       = layer->texture;
   data.type          = layer->type;
   return data;
}
std::optional<RaceTintLayerModel::LayerData> RaceTintLayerModel::getLayerData(dovah::face_tint_index_type li) const {
   auto* layer = this->_layer_by_index(li);
   if (layer) {
      LayerData data;
      data.default_color = layer->default_color;
      data.texture       = layer->texture;
      data.type          = layer->type;
      return data;
   }
   return {};
}
void RaceTintLayerModel::setLayerData(const QModelIndex& qmi, const LayerData& src) {
   auto* layer = _layer_from_qmi(qmi);
   if (!layer)
      return;
   layer->default_color = src.default_color;
   layer->texture       = src.texture;
   layer->type          = src.type;
   
   auto tl = qmi.siblingAtColumn(0);
   auto br = qmi.siblingAtColumn(ColumnCount);
   emit dataChanged(tl, br);
}
void RaceTintLayerModel::setLayerData(dovah::face_tint_index_type li, const LayerData& src) {
   auto row = this->_row_for_layer(li);
   if (!row.has_value())
      return;
   auto* dst = this->_data.layers[row.value()];
   dst->default_color = src.default_color;
   dst->texture = src.texture;
   dst->type    = src.type;

   auto tl = this->index(row.value(), 0, {});
   auto br = tl.siblingAtColumn(ColumnCount);
   emit dataChanged(tl, br);
}

[[nodiscard]] std::optional<RaceTintLayerModel::LayerPreset> RaceTintLayerModel::getLayerPresetByIndex(dovah::face_tint_index_type preset_index) {
   auto* src = this->_preset_by_index(preset_index);
   if (src)
      return *src;
   return {};
}
[[nodiscard]] std::optional<RaceTintLayerModel::LayerPreset> RaceTintLayerModel::getLayerPresetByIndex(dovah::face_tint_index_type layer_index, dovah::face_tint_index_type preset_index) {
   auto* src = this->_preset_by_indices(layer_index, preset_index);
   if (src)
      return *src;
   return {};
}
[[nodiscard]] std::optional<RaceTintLayerModel::LayerPreset> RaceTintLayerModel::getLayerPresetByRow(dovah::face_tint_index_type layer_index, size_t row) {
   if (auto* layer = this->_layer_by_index(layer_index)) {
      if (row < layer->presets.size())
         return layer->presets[row];
   }
   return {};
}
std::optional<dovah::face_tint_index_type> RaceTintLayerModel::addLayerPreset(dovah::face_tint_index_type layer_index) { // returns new preset's index
   QModelIndex qmi;
   Layer*      layer = nullptr;
   for (size_t i = 0; i < this->_data.layers.size(); ++i) {
      auto* item = this->_data.layers[i];
      if (item->index == layer_index) {
         qmi   = this->index(i, 0, {});
         layer = item;
         break;
      }
   }
   if (!layer)
      return {};

   this->beginInsertRows(qmi, layer->presets.size(), layer->presets.size());
   auto& dst = layer->presets.emplace_back();
   dst.index = this->_allocate_new_index();
   dst.alpha = 1;
   this->endInsertRows();

   return dst.index;
}
void RaceTintLayerModel::replaceLayerPreset(const LayerPreset& src) {
   if (!this->_index_is_in_use(src.index))
      return;
   for (size_t lr = 0; lr < this->_data.layers.size(); ++lr) {
      auto* layer = this->_data.layers[lr];
      for (size_t pr = 0; pr < layer->presets.size(); ++pr) {
         auto& preset = layer->presets[pr];
         if (preset.index == src.index) {
            preset = src;

            auto l_qmi = this->index(lr, 0, {});
            auto p_qmi = this->index(pr, 0, l_qmi);
            emit dataChanged(p_qmi, p_qmi.siblingAtColumn(ColumnCount));
            return;
         }
      }
   }
}
void RaceTintLayerModel::replaceLayerPreset(dovah::face_tint_index_type li, const LayerPreset& src) {
   if (!this->_index_is_in_use(li))
      return;
   if (!this->_index_is_in_use(src.index))
      return;
   for (size_t lr = 0; lr < this->_data.layers.size(); ++lr) {
      auto* layer = this->_data.layers[lr];
      if (layer->index != li)
         continue;
      for (size_t pr = 0; pr < layer->presets.size(); ++pr) {
         auto& preset = layer->presets[pr];
         if (preset.index == src.index) {
            preset = src;

            auto l_qmi = this->index(lr, 0, {});
            auto p_qmi = this->index(pr, 0, l_qmi);
            emit dataChanged(p_qmi, p_qmi.siblingAtColumn(ColumnCount));
            return;
         }
      }
   }
}
void RaceTintLayerModel::removeLayerPresetByIndex(dovah::face_tint_index_type layer_index, dovah::face_tint_index_type preset_index) {
   auto layer_row = this->_row_for_layer(layer_index);
   if (!layer_row.has_value())
      return;
   auto  layer_qmi = this->index(layer_row.value(), 0, {});
   auto* layer     = this->_data.layers[layer_row.value()];
   for(size_t pr = 0; pr < layer->presets.size(); ++pr) {
      auto& preset = layer->presets[pr];
      if (preset.index != preset_index)
         continue;
      this->beginRemoveRows(layer_qmi, pr, pr);
      layer->presets.erase(layer->presets.begin() + pr);
      this->_try_free_index(preset.index);
      this->endRemoveRows();
      return;
   }
}
void RaceTintLayerModel::removeLayerPresetByRow(dovah::face_tint_index_type layer_index, size_t preset_row) {
   auto layer_row = this->_row_for_layer(layer_index);
   if (!layer_row.has_value())
      return;
   auto  layer_qmi = this->index(layer_row.value(), 0, {});
   auto* layer     = this->_data.layers[layer_row.value()];
   if (preset_row >= layer->presets.size())
      return;
   this->beginRemoveRows(layer_qmi, preset_row, preset_row);
   auto pi = layer->presets[preset_row].index;
   layer->presets.erase(layer->presets.begin() + preset_row);
   this->_try_free_index(pi);
   this->endRemoveRows();
}

std::optional<dovah::face_tint_index_type> RaceTintLayerModel::layerIndex(const QModelIndex& qmi) const {
   if (!_qmi_is_layer(qmi))
      return {};
   auto* layer =_layer_from_qmi(qmi);
   if (!layer)
      return {};
   return layer->index;
}
std::optional<dovah::face_tint_index_type> RaceTintLayerModel::layerIndex(size_t row) const {
   if (row >= this->rowCount())
      return {};
   auto* layer = this->_data.layers[row];
   if (!layer)
      return {};
   return layer->index;
}

[[nodiscard]] std::optional<RaceTintLayerModel::Layer> RaceTintLayerModel::layerDefinitionByIndex(dovah::face_tint_index_type layer_index) const {
   auto* layer = this->_layer_by_index(layer_index);
   if (layer)
      return *layer;
   return {};
}

#pragma region RaceTintLayerPresetsModel
/*virtual*/ QVariant RaceTintLayerPresetsModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
   if (orientation != Qt::Orientation::Vertical)
      return {};
   if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
      return {};
   switch (section) {
      case Column::ColorPreview:
         break;
      case Column::Name:
         return tr("Color");
      case Column::Alpha:
         return tr("Alpha");
   }
   return {};
}
#pragma endregion