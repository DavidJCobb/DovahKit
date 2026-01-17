#include "./RegionSoundsModel.h"
#include "dovah/forms/Region.h"
#include "dovah/form_stub.h"
#include "editor/core.h"
#include "editor/form_stub_meta_type.h"
#include "editor/helpers/form_stub_drag_drop.h"
#include "ui/types/regions/region.h"

RegionSoundsModel::RegionSoundsModel(QObject* parent) : QAbstractItemModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub) { this->_on_form_deleted(*stub); });
   QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) { this->_on_form_modified(*stub); });
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() { this->clear(); });
}
RegionSoundsModel::~RegionSoundsModel() {
}

#pragma region QAbstractItemModel /*override*/s
   #pragma region Hierarchy
      /*virtual*/ QModelIndex RegionSoundsModel::index(int row, int column, const QModelIndex& parent) const /*override*/ {
         if (row < 0 || column < 0)
            return {};
         if (row >= this->_items.size() || column >= ColumnCount)
            return {};
         if (parent.isValid())
            return {};
         return createIndex(row, column, nullptr);
      }
      /*virtual*/ QModelIndex RegionSoundsModel::parent(const QModelIndex& index) const /*override*/ {
         return {};
      }
      /*virtual*/ QModelIndex RegionSoundsModel::sibling(int row, int column, const QModelIndex& qmi) const /*override*/ {
         return index(row, column, {});
      }
      /*virtual*/ int RegionSoundsModel::rowCount(const QModelIndex& parent) const /*override*/ {
         return this->_items.size();
      }
      /*virtual*/ int RegionSoundsModel::columnCount(const QModelIndex& parent) const /*override*/ {
         return ColumnCount;
      }
      #pragma region Editing
         /*virtual*/ bool RegionSoundsModel::insertRows(int row, int count, const QModelIndex& parent) /*override*/ {
            if (row < 0 || row > this->_items.size() || count <= 0)
               return false;
            if (parent.isValid())
               return false;
            this->beginInsertRows(parent, row, row + count - 1);
            auto it = this->_items.begin() + row;
            this->_items.insert(it, count, {});
            this->endRemoveRows();
            return true;
         }
         /*virtual*/ bool RegionSoundsModel::removeRows(int row, int count, const QModelIndex& parent) /*override*/ {
            if (row < 0 || row >= this->_items.size() || count <= 0)
               return false;
            if (parent.isValid())
               return false;
            this->beginRemoveRows({}, row, row + count - 1);
            auto a = this->_items.begin() + row;
            auto b = a + count;
            this->_items.erase(a, b);
            this->endRemoveRows();
            return true;
         }
      #pragma endregion
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant RegionSoundsModel::data(const QModelIndex& qmi, int role) const /*override*/ {
         if (!qmi.isValid() || qmi.model() != this)
            return {};
         auto row = qmi.row();
         auto col = qmi.column();
         if (row >= this->_items.size())
            return {};
         if (col >= ColumnCount)
            return {};
         auto& item = this->_items[row];
         switch (role) {
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
               switch (col) {
                  case Column::SoundName:
                     return item.cached.editor_id;
                  case Column::Chance:
                     return item.chance;
               }
               break;
            case Qt::CheckStateRole:
               {
                  bool checked = false;
                  switch (col) {
                     case Column::WeatherIsPleasant:
                        checked = item.weather.pleasant;
                        break;
                     case Column::WeatherIsCloudy:
                        checked = item.weather.cloudy;
                        break;
                     case Column::WeatherIsRainy:
                        checked = item.weather.rainy;
                        break;
                     case Column::WeatherIsSnowy:
                        checked = item.weather.snowy;
                        break;
                     default:
                        return {};
                  }
                  return checked ? Qt::CheckState::Checked : Qt::CheckState::Unchecked;
               }
               break;
            case FormStubRole:
               return QVariant::fromValue<dovah::form_stub*>(item.form);
            case Qt::EditRole:
               switch (col) {
                  case Column::SoundName:
                     return QVariant::fromValue<dovah::form_stub*>(item.form);
                  case Column::Chance:
                     return item.chance;
                  case Column::WeatherIsPleasant:
                     return item.weather.pleasant;
                  case Column::WeatherIsCloudy:
                     return item.weather.cloudy;
                  case Column::WeatherIsRainy:
                     return item.weather.rainy;
                  case Column::WeatherIsSnowy:
                     return item.weather.snowy;
               }
               break;
         }
         return {};
      }
      /*virtual*/ Qt::ItemFlags RegionSoundsModel::flags(const QModelIndex& qmi) const /*override*/ {
         if (!qmi.isValid())
            return Qt::ItemFlag::ItemIsDropEnabled;
         auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
         if (qmi.column() >= Column::WeatherIsPleasant && qmi.column() <= Column::WeatherIsSnowy)
            flags |= Qt::ItemFlag::ItemIsUserCheckable;
         return flags;
      }
      /*virtual*/ bool RegionSoundsModel::setData(const QModelIndex& qmi, const QVariant& value, int role) /*override*/ {
         if (!qmi.isValid())
            return false;
         auto i = qmi.row();
         if (i >= this->_items.size())
            return false;
         auto& item = this->_items[i];
         switch (role) {
            case FormStubRole:
               if (auto* stub = value.value<dovah::form_stub*>()) {
                  if (stub->form_type != dovah::form_type::sound_descriptor)
                     return false;
                  if (item.form != stub) {
                     item.form = stub;
                     item.cached.editor_id = QString::fromStdString(stub->editorID);
                     const auto name_qmi = index(i, Column::SoundName, {});
                     emit dataChanged(name_qmi, name_qmi);
                  }
                  return true;
               }
               return false;
            case Qt::EditRole:
               switch (qmi.column()) {
                  case Column::SoundName:
                     return setData(qmi, value, FormStubRole);
                  case Column::Chance:
                     if (value.canConvert<float>()) {
                        item.chance = value.value<float>();
                        emit dataChanged(qmi, qmi);
                        return true;
                     }
                     return false;
                  case Column::WeatherIsPleasant:
                     item.weather.pleasant = value.toBool();
                     emit dataChanged(qmi, qmi);
                     return true;
                  case Column::WeatherIsCloudy:
                     item.weather.cloudy = value.toBool();
                     emit dataChanged(qmi, qmi);
                     return true;
                  case Column::WeatherIsRainy:
                     item.weather.rainy = value.toBool();
                     emit dataChanged(qmi, qmi);
                     return true;
                  case Column::WeatherIsSnowy:
                     item.weather.snowy = value.toBool();
                     emit dataChanged(qmi, qmi);
                     return true;
               }
               break;
         }
         return false;
      }
   #pragma endregion
   /*virtual*/ QVariant RegionSoundsModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
      if (orientation != Qt::Orientation::Horizontal)
         return {};
      switch (role) {
         case Qt::DisplayRole:
         case Qt::ToolTipRole:
            break;
         default:
            return {};
      }
      switch (section) {
         case Column::SoundName: return tr("Sound");
         case Column::Chance: return tr("Chance");
         case Column::WeatherIsPleasant: return tr("Is Pleasant");
         case Column::WeatherIsCloudy: return tr("Is Cloudy");
         case Column::WeatherIsRainy: return tr("Is Rainy");
         case Column::WeatherIsSnowy: return tr("Is Snowy");
      }
      return {};
   }
   #pragma region Drag and drop
      #pragma region Whole-model queries
         /*virtual*/ QStringList RegionSoundsModel::mimeTypes() const /*override*/ {
            return QStringList(QString(editor_helpers::form_stub_array_mime_type));
         }
         /*virtual*/ Qt::DropActions RegionSoundsModel::supportedDropActions() const /*override*/ {
            return Qt::CopyAction;
         }
      #pragma endregion
      /*virtual*/ bool RegionSoundsModel::canDropMimeData(const QMimeData* data, Qt::DropAction, int row, int column, const QModelIndex& parent) const /*override*/ {
         if (!data->hasFormat(editor_helpers::form_stub_array_mime_type))
            return false;

         auto list = editor_helpers::form_stubs_from_mime_data(*data);
         for (auto* stub : list) {
            if (stub && stub->form_type == dovah::form_type::sound_descriptor) {
               return true;
            }
         }
         return false;
      }
      /*virtual*/ bool RegionSoundsModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) /*override*/ {
         if (!this->canDropMimeData(data, action, row, column, parent))
            return false;
         if (action == Qt::IgnoreAction)
            return true;
         if (row == -1) {
            row = this->_items.size();
         }
         
         auto dropped_stubs = editor_helpers::form_stubs_from_mime_data(*data);
         std::erase_if(
            dropped_stubs,
            [](dovah::form_stub* stub) -> bool {
               return stub && stub->form_type == dovah::form_type::sound_descriptor;
            }
         );
         if (dropped_stubs.empty())
            return false;

         this->beginInsertRows({}, row, row + dropped_stubs.size() - 1);
         for (size_t i = 0; i < dropped_stubs.size(); ++i) {
            this->_items.insert(this->_items.begin() + row + i, {});
            auto& item = this->_items[row + i];
            item.form = dropped_stubs[i];
            item.cached.editor_id = QString::fromStdString(item.form->editorID);
         }
         this->endInsertRows();
         return true;
      }
   #pragma endregion
#pragma endregion

void RegionSoundsModel::importData(const frontend_form_data& region) {
   this->beginResetModel();
   this->_items.clear();
   
   auto& src_coll_opt = region.generable_content.audio;
   if (src_coll_opt.has_value()) {
      auto& src_coll = src_coll_opt.value();
      for (auto& src_item : src_coll.ambient_sounds) {
         if (!src_item.sound || src_item.sound->form_type != dovah::form_type::sound_descriptor)
            continue;
         auto& item = this->_items.emplace_back();
         item.form   = src_item.sound;
         item.chance = src_item.chance;
         item.weather.pleasant = src_item.weather.pleasant;
         item.weather.cloudy   = src_item.weather.cloudy;
         item.weather.rainy    = src_item.weather.rainy;
         item.weather.snowy    = src_item.weather.snowy;
         //
         item.cached.editor_id = QString::fromStdString(item.form->editorID);
      }
   }

   this->endResetModel();
}
void RegionSoundsModel::exportData(frontend_form_data& region) const {
   auto& dst_coll_opt = region.generable_content.audio;
   if (dst_coll_opt.has_value()) {
      dst_coll_opt.value().ambient_sounds.clear();
   } else {
      dst_coll_opt.emplace();
   }

   auto& dst_coll = dst_coll_opt.value();
   dst_coll.ambient_sounds.reserve(this->_items.size());
   for (auto& src_item : this->_items) {
      auto& dst_item = dst_coll.ambient_sounds.emplace_back();
      dst_item.sound   = src_item.form;
      dst_item.chance  = src_item.chance;
      dst_item.weather.pleasant = src_item.weather.pleasant;
      dst_item.weather.cloudy   = src_item.weather.cloudy;
      dst_item.weather.rainy    = src_item.weather.rainy;
      dst_item.weather.snowy    = src_item.weather.snowy;
   }
}
void RegionSoundsModel::clear() {
   this->beginResetModel();
   this->_items.clear();
   this->endResetModel();
}

void RegionSoundsModel::_on_form_deleted(dovah::form_stub& stub) {
   if (stub.form_type != dovah::form_type::sound_descriptor)
      return;
   auto&  list = this->_items;
   size_t size = list.size();
   for (size_t i = 0; i < size; ++i) {
      if (list[i].form != &stub)
         continue;
      this->beginRemoveRows({}, i, i);
      list.erase(list.begin() + i);
      --i;
      --size;
      this->endRemoveRows();
   }
}
void RegionSoundsModel::_on_form_modified(dovah::form_stub& stub) {
   if (stub.form_type != dovah::form_type::sound_descriptor)
      return;
   QString editor_id;
   for (size_t i = 0; i < this->_items.size(); ++i) {
      auto& item = this->_items[i];
      if (item.form != &stub)
         continue;
      if (editor_id.isEmpty()) {
         editor_id = QString::fromStdString(stub.editorID);
      }
      item.cached.editor_id = editor_id;
      auto qmi = this->index(i, Column::SoundName, {});
      emit dataChanged(qmi, qmi);
   }
}