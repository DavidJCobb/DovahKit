#include "./ActorBaseTintLayerModel.h"
#include <QBrush>
#include "dovah/forms/ActorBase.h"
#include "dovah/forms/Color.h"
#include "dovah/forms/Race.h"
#include "editor/core.h"

const ActorBaseTintLayerModel::TintLayerPreset* ActorBaseTintLayerModel::TintLayer::preset_by_index(uint16_t index) const {
   for (auto& preset : this->presets)
      if (preset.index == index)
         return &preset;
   return nullptr;
}
void ActorBaseTintLayerModel::TintLayer::recache_color(dovah::form_stub& stub) {
   if (stub.form_type != dovah::form_type::color)
      return;
   dovah::loaded_form_ptr<dovah::loaded_forms::Color> loaded;
   for (auto& preset : this->presets) {
      if (preset.color.form != &stub)
         continue;
      if (!loaded) {
         loaded = stub.load().ptr_cast<dovah::loaded_forms::Color>();
         if (!loaded)
            return;
      }
      preset.color.cached = QColor(loaded->color.r, loaded->color.g, loaded->color.b);
   }
}
void ActorBaseTintLayerModel::TintLayer::recache_all_colors() {
   for (auto& preset : this->presets) {
      auto* stub = preset.color.form;
      if (!stub)
         continue;
      if (stub->form_type != dovah::form_type::color)
         continue;
      auto loaded = stub->load().ptr_cast<dovah::loaded_forms::Color>();
      if (!loaded)
         continue;
      preset.color.cached = QColor(loaded->color.r, loaded->color.g, loaded->color.b);
   }
}

ActorBaseTintLayerModel::ActorBaseTintLayerModel(QObject* parent) : QAbstractItemModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, &ActorBaseTintLayerModel::_clear);
   QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) {
      if (stub->form_type == dovah::form_type::color) {
         for (auto& by_sex : this->_data.layers) {
            for (auto& layer : by_sex)
               layer->recache_color(*stub);
         }
      }
   });
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub, bool just_being_flagged) {
      if (stub->form_type == dovah::form_type::color) {
         for (auto& by_sex : this->_data.layers) {
            for (auto* layer : by_sex) {
               for (auto& preset : layer->presets) {
                  if (preset.color.form == stub)
                     preset.color = {}; // TODO: Can the color of a preset be nullptr, or should we remove the preset?
               }
               if (layer->default_color == stub)
                  layer->default_color = nullptr; // can be null
            }
         }
      }
   });
}
ActorBaseTintLayerModel::~ActorBaseTintLayerModel() {
   this->_clear_silent();
}

void ActorBaseTintLayerModel::_clear_all_layers() {
   for (auto& by_sex : this->_data.layers) {
      for (auto* layer : by_sex)
         delete layer;
      by_sex.clear();
   }
}
void ActorBaseTintLayerModel::_clear_all_states() {
   for (auto* state : this->_data.states)
      delete state;
   this->_data.states.clear();
}
void ActorBaseTintLayerModel::_clear_silent() {
   this->_clear_all_states();
   this->_clear_all_layers();
}
void ActorBaseTintLayerModel::_clear() {
   this->beginResetModel();
   this->_clear_silent();
   this->endResetModel();
}

#pragma region QAbstractItemModel /*override*/s
   #pragma region Hierarchy
      /*virtual*/ QModelIndex ActorBaseTintLayerModel::index(int row, int col, const QModelIndex& parent) const /*override*/ {
         if (row < 0 || row >= this->rowCount())
            return {};
         if (col < 0 || col >= ColumnCount)
            return {};
         if (parent.isValid())
            return {};
         return this->createIndex(row, col, nullptr);
      }
      /*virtual*/ QModelIndex ActorBaseTintLayerModel::parent(const QModelIndex& index) const /*override*/ {
         return {};
      }
      /*virtual*/ QModelIndex ActorBaseTintLayerModel::sibling(int row, int column, const QModelIndex& index) const /*override*/ {
         if (!index.isValid())
            return {};
         return this->index(row, column, {});
      }
      /*virtual*/ int ActorBaseTintLayerModel::rowCount(const QModelIndex& parent) const /*override*/ {
         return this->_data.layers[this->_data.sex].size();
      }
      /*virtual*/ int ActorBaseTintLayerModel::columnCount(const QModelIndex& parent) const /*override*/ {
         return ColumnCount;
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant ActorBaseTintLayerModel::data(const QModelIndex& index, int role) const /*override*/ {
         auto& layers = this->_data.layers[this->_data.sex];
         auto  row    = index.row();
         if (row < 0 || row >= layers.size())
            return {};
         const auto* layer = layers[row];
         if (!layer)
            return {};

         const TintLayerActorState* state = this->layerStateByIndex(layer->index);
         
         switch (index.column()) {
            case Column::TexturePath:
               switch (role) {
                  case Qt::DisplayRole:
                  case Qt::ToolTipRole:
                     return layer->texture;
               }
               break;
            case Column::Color:
               if (!state)
                  return {};
               switch (role) {
                  case Qt::BackgroundRole:
                     if (!dovah::face_tint_index_is_none(state->preset_index)) {
                        auto* preset = layer->preset_by_index(state->preset_index);
                        if (preset)
                           return QBrush(preset->color.cached);
                     }
                     return QBrush(state->color);
               }
               break;
            case Column::Alpha:
               if (!state)
                  return {};
               switch (role) {
                  case Qt::DisplayRole:
                  case Qt::ToolTipRole:
                     return QString("%1%").arg(state->alpha);
                  case Qt::TextAlignmentRole:
                     return (int)(Qt::AlignVCenter | Qt::AlignRight);
               }
               break;
         }
         return {};
      }
      /*virtual*/ Qt::ItemFlags ActorBaseTintLayerModel::flags(const QModelIndex& index) const /*override*/ {
         if (!index.isValid())
            return {};
         return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
      }
   #pragma endregion
   /*virtual*/ QVariant ActorBaseTintLayerModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
      if (orientation != Qt::Orientation::Horizontal)
         return {};
      switch (role) {
         case Qt::DisplayRole:
         case Qt::ToolTipRole:
            switch (section) {
               case Column::TexturePath:
                  return tr("Texture");
               case Column::Color:
                  return tr("Color");
               case Column::Alpha:
                  return tr("Alpha %");
            }
            break;
      }
      return {};
   }
#pragma endregion

void ActorBaseTintLayerModel::importLayerDefinitions(const dovah::loaded_forms::Race& race) {
   this->beginResetModel();
   this->_clear_all_layers();
   for (size_t i = 0; i < dovah::sex_count; ++i) {
      auto sex = (dovah::sex)i;
      
      auto& src = race.by_sex[sex].head_data.face_tints;
      auto& dst = this->_data.layers[sex];

      for (auto& src_layer : src) {
         auto& dst_layer = dst.emplace_back();
         dst_layer = new TintLayer;
         dst_layer->index   = src_layer.index;
         dst_layer->texture = QString::fromStdString(src_layer.texture);
         dst_layer->type    = src_layer.type;
         for (auto& src_preset : src_layer.presets) {
            auto& dst_preset = dst_layer->presets.emplace_back();
            dst_preset.alpha = src_preset.alpha;
            dst_preset.index = src_preset.index;
            dst_preset.color.form = src_preset.color.get_form_stub();
         }
         dst_layer->default_color = src_layer.default_color.get_form_stub();
         dst_layer->recache_all_colors();
      }
   }
   this->endResetModel();
}
void ActorBaseTintLayerModel::importLayerStates(const dovah::loaded_forms::ActorBase& actor_base) {
   this->_clear_all_states();

   auto sex = actor_base.get_local_sex();
   this->_data.sex = sex;

   for (auto& src : actor_base.tint_layers) {
      auto& dst = this->_data.states.emplace_back();
      dst = new TintLayerActorState;
      dst->alpha        = src.interpolation;
      dst->layer_index  = src.index;
      dst->preset_index = src.preset;
      dst->color        = QColor(src.color.r, src.color.g, src.color.b);
   }

   auto tl = this->index(0, Column::Color, {});
   auto br = this->index(this->_data.layers.size(), Column::Alpha, {});
   emit dataChanged(tl, br);
}
void ActorBaseTintLayerModel::importLayerState(const TintLayerActorState& state) {
   bool exists = false;
   for (auto* existing : this->_data.states) {
      if (existing->layer_index == state.layer_index) {
         *existing = state;
         exists = true;
         break;
      }
   }
   if (!exists) {
      auto*& ptr = this->_data.states.emplace_back();
      ptr = new TintLayerActorState(state);
   }

   auto& layer_list = this->_data.layers[this->_data.sex];
   for (size_t i = 0; i < layer_list.size(); ++i) {
      if (layer_list[i]->index == state.layer_index) {
         auto tl = this->index(i, Column::Color, {});
         auto br = this->index(i, Column::Alpha, {});
         emit dataChanged(tl, br);
         break;
      }
   }
}
void ActorBaseTintLayerModel::setSex(dovah::sex s) {
   if (this->_data.sex == s)
      return;
   emit this->beginResetModel();
   this->_data.sex = s;
   emit this->endResetModel();
}

void ActorBaseTintLayerModel::exportLayerStates(dovah::loaded_forms::ActorBase& dst) {
   dst.tint_layers.clear();

   const auto& layer_definitions = this->_data.layers[this->_data.sex];
   for (auto* src_item : this->_data.states) {
      auto& dst_item = dst.tint_layers.emplace_back();
      dst_item.interpolation = src_item->alpha;
      dst_item.index = src_item->layer_index;

      QColor src_color = src_item->color;
      if (dovah::face_tint_index_is_none(src_item->preset_index)) {
         dst_item.preset = dovah::index_of_no_face_tint;
      } else {
         dst_item.preset = src_item->preset_index;
         for (auto* layer_dfn : layer_definitions) {
            if (layer_dfn->index == src_item->layer_index) {
               for (auto& preset : layer_dfn->presets) {
                  if (preset.index == src_item->preset_index) {
                     src_color = preset.color.cached;
                  }
               }
               break;
            }
         }
      }
      auto& dst_color = dst_item.color;
      dst_color.r = src_color.red();
      dst_color.g = src_color.green();
      dst_color.b = src_color.blue();
   }
}

std::optional<uint16_t> ActorBaseTintLayerModel::layerIndex(const QModelIndex& qmi) const {
   if (!qmi.isValid())
      return {};
   auto row = qmi.row();
   if (row < 0)
      return {};
   return this->layerIndex(row);
}
std::optional<uint16_t> ActorBaseTintLayerModel::layerIndex(size_t row) const {
   if (row >= this->rowCount())
      return {};
   auto& layers = this->_data.layers[this->_data.sex];
   auto* layer = layers[row];
   if (!layer)
      return {};
   return layer->index;
}

[[nodiscard]] std::optional<ActorBaseTintLayerModel::TintLayer> ActorBaseTintLayerModel::layerDefinitionByIndex(uint16_t layer_index) const {
   for (auto* layer : this->_data.layers[this->_data.sex]) {
      if (layer->index == layer_index)
         return *layer;
   }
   return {};
}
const ActorBaseTintLayerModel::TintLayerActorState* ActorBaseTintLayerModel::layerStateByIndex(uint16_t index) const {
   for (auto* state : this->_data.states)
      if (state->layer_index == index)
         return state;
   return nullptr;
}