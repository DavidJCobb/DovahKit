#include "./RegionWeatherModel.h"
#include "dovah/form_stub.h"
#include "editor/core.h"
#include "editor/form_stub_meta_type.h"
#include "editor/helpers/form_stub_drag_drop.h"
#include "ui/types/regions/region.h"

RegionWeatherModel::RegionWeatherModel(QObject* parent) : QAbstractItemModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub) { this->_on_form_deleted(*stub); });
   QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) { this->_on_form_modified(*stub); });
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() { this->clear(); });
}
RegionWeatherModel::~RegionWeatherModel() {
}

#pragma region QAbstractItemModel /*override*/s
   #pragma region Hierarchy
      /*virtual*/ QModelIndex RegionWeatherModel::index(int row, int column, const QModelIndex& parent) const /*override*/ {
         if (row < 0 || column < 0)
            return {};
         if (row >= this->_items.size() || column >= ColumnCount)
            return {};
         if (parent.isValid())
            return {};
         return createIndex(row, column, nullptr);
      }
      /*virtual*/ QModelIndex RegionWeatherModel::parent(const QModelIndex& index) const /*override*/ {
         return {};
      }
      /*virtual*/ QModelIndex RegionWeatherModel::sibling(int row, int column, const QModelIndex& qmi) const /*override*/ {
         return index(row, column, {});
      }
      /*virtual*/ int RegionWeatherModel::rowCount(const QModelIndex& parent) const /*override*/ {
         return this->_items.size();
      }
      /*virtual*/ int RegionWeatherModel::columnCount(const QModelIndex& parent) const /*override*/ {
         return ColumnCount;
      }
      #pragma region Editing
         /*virtual*/ bool RegionWeatherModel::removeRows(int row, int count, const QModelIndex& parent) /*override*/ {
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
      /*virtual*/ QVariant RegionWeatherModel::data(const QModelIndex& qmi, int role) const /*override*/ {
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
                  case Column::WeatherName:
                     return item.cached.weather_id;
                  case Column::Chance:
                     return item.chance;
                  case Column::GlobalName:
                     return item.cached.global_id;
               }
               break;
            case WeatherStubRole:
               return QVariant::fromValue<dovah::form_stub*>(item.weather);
            case GlobalStubRole:
               return QVariant::fromValue<dovah::form_stub*>(item.global);
            case Qt::EditRole:
               switch (col) {
                  case Column::WeatherName:
                     return QVariant::fromValue<dovah::form_stub*>(item.weather);
                  case Column::Chance:
                     return item.chance;
                  case Column::GlobalName:
                     return QVariant::fromValue<dovah::form_stub*>(item.global);
               }
               break;
         }
         return {};
      }
      /*virtual*/ Qt::ItemFlags RegionWeatherModel::flags(const QModelIndex& qmi) const /*override*/ {
         if (!qmi.isValid())
            return Qt::ItemFlag::ItemIsDropEnabled;
         auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
         return flags;
      }
      /*virtual*/ bool RegionWeatherModel::setData(const QModelIndex& qmi, const QVariant& value, int role) /*override*/ {
         if (!qmi.isValid())
            return false;
         auto i = qmi.row();
         if (i >= this->_items.size())
            return false;
         auto& item = this->_items[i];
         switch (role) {
            case WeatherStubRole:
               if (auto* stub = value.value<dovah::form_stub*>()) {
                  if (stub->form_type != dovah::form_type::weather)
                     return false;
                  if (this->containsWeather(*stub))
                     return false;
                  if (item.weather != stub) {
                     item.weather = stub;
                     item.cached.weather_id = QString::fromStdString(stub->editorID);
                     const auto name_qmi = index(i, Column::WeatherName, {});
                     emit dataChanged(name_qmi, name_qmi);
                  }
                  return true;
               }
               return false;
            case GlobalStubRole:
               if (auto* stub = value.value<dovah::form_stub*>()) {
                  if (stub->form_type != dovah::form_type::global)
                     return false;
                  if (item.global != stub) {
                     item.global = stub;
                     item.cached.global_id = QString::fromStdString(stub->editorID);
                     const auto name_qmi = index(i, Column::GlobalName, {});
                     emit dataChanged(name_qmi, name_qmi);
                  }
                  return true;
               }
               return false;
            case Qt::EditRole:
               switch (qmi.column()) {
                  case Column::WeatherName:
                     return setData(qmi, value, WeatherStubRole);
                  case Column::Chance:
                     if (value.canConvert<float>()) {
                        item.chance = value.value<float>();
                        emit dataChanged(qmi, qmi);
                        return true;
                     }
                     return false;
                  case Column::GlobalName:
                     return setData(qmi, value, GlobalStubRole);
               }
               break;
         }
         return false;
      }
   #pragma endregion
   /*virtual*/ QVariant RegionWeatherModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
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
         case Column::WeatherName: return tr("Weather");
         case Column::Chance: return tr("Chance");
         case Column::GlobalName: return tr("Global");
      }
      return {};
   }
   #pragma region Drag and drop
      #pragma region Whole-model queries
         /*virtual*/ QStringList RegionWeatherModel::mimeTypes() const /*override*/ {
            return QStringList(QString(editor_helpers::form_stub_array_mime_type));
         }
         /*virtual*/ Qt::DropActions RegionWeatherModel::supportedDropActions() const /*override*/ {
            return Qt::CopyAction;
         }
      #pragma endregion
      /*virtual*/ bool RegionWeatherModel::canDropMimeData(const QMimeData* data, Qt::DropAction, int row, int column, const QModelIndex& parent) const /*override*/ {
         if (!data->hasFormat(editor_helpers::form_stub_array_mime_type))
            return false;

         auto list = editor_helpers::form_stubs_from_mime_data(*data);
         for (auto* stub : list) {
            if (stub && stub->form_type == dovah::form_type::weather && !this->containsWeather(*stub)) {
               return true;
            }
         }
         return false;
      }
      /*virtual*/ bool RegionWeatherModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) /*override*/ {
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
            [this](dovah::form_stub* stub) -> bool {
               return !stub || stub->form_type != dovah::form_type::weather || this->containsWeather(*stub);
            }
         );
         if (dropped_stubs.empty())
            return false;

         this->beginInsertRows({}, row, row + dropped_stubs.size() - 1);
         for (size_t i = 0; i < dropped_stubs.size(); ++i) {
            auto* stub = dropped_stubs[i];
            this->_items.insert(
               this->_items.begin() + row + i,
               Item{
                  .weather = stub,
                  .cached  = {
                     .weather_id = QString::fromStdString(stub->editorID),
                  }
               }
            );
         }
         this->endInsertRows();
         return true;
      }
   #pragma endregion
#pragma endregion

void RegionWeatherModel::importData(const frontend_form_data& region) {
   this->beginResetModel();
   this->_items.clear();

   auto& src_coll_opt = region.generable_content.weather;
   if (src_coll_opt.has_value()) {
      auto& src_coll = src_coll_opt.value();
      for (auto& src_item : src_coll.weathers) {
         if (!src_item.weather || src_item.weather->form_type != dovah::form_type::weather)
            continue;
         if (this->containsWeather(*src_item.weather))
            continue;
         auto& dst_item = this->_items.emplace_back();
         dst_item.weather = src_item.weather;
         dst_item.chance  = src_item.chance_constant;
         dst_item.global  = src_item.chance_global;
         
         dst_item.cached.weather_id = QString::fromStdString(dst_item.weather->editorID);
         if (dst_item.global)
            dst_item.cached.global_id = QString::fromStdString(dst_item.global->editorID);
      }
   }

   this->endResetModel();
}
void RegionWeatherModel::exportData(frontend_form_data& region) const {
   auto& dst_coll_opt = region.generable_content.weather;
   dst_coll_opt.emplace();

   auto& dst_coll = dst_coll_opt.value();
   dst_coll.weathers.reserve(this->_items.size());
   for (auto& src_item : this->_items) {
      auto& dst_item = dst_coll.weathers.emplace_back();
      dst_item.weather         = src_item.weather;
      dst_item.chance_constant = src_item.chance;
      dst_item.chance_global   = src_item.global;
   }
}
void RegionWeatherModel::clear() {
   this->beginResetModel();
   this->_items.clear();
   this->endResetModel();
}

QModelIndex RegionWeatherModel::addWeather(dovah::form_stub& weather, float chance, dovah::form_stub* global) {
   const auto size = this->_items.size();
   for (size_t i = 0; i < size; ++i) {
      auto& item = this->_items[i];
      if (item.weather == &weather) {
         if (item.chance != chance) {
            item.chance = chance;
            auto qmi = this->index(i, Column::Chance, {});
            emit dataChanged(qmi, qmi);
         }
         if (item.global != global) {
            item.global = global;
            auto qmi = this->index(i, Column::GlobalName, {});
            emit dataChanged(qmi, qmi);
         }
         return this->index(i, 0, {});
      }
   }
   this->beginInsertRows({}, size, size);
   auto& item = this->_items.emplace_back();
   item.weather = &weather;
   item.chance  = chance;
   item.global  = global;
   item.cached.weather_id = QString::fromStdString(weather.editorID);
   if (global)
      item.cached.global_id = QString::fromStdString(global->editorID);
   this->endInsertRows();
   return this->index(size, 0, {});
}
bool RegionWeatherModel::containsWeather(const dovah::form_stub& stub) const {
   if (stub.form_type != dovah::form_type::weather)
      return false;
   for (auto& item : this->_items)
      if (item.weather == &stub)
         return true;
   return false;
}

std::vector<dovah::form_stub*> RegionWeatherModel::allWeathers() const {
   std::vector<dovah::form_stub*> out;
   for (auto& item : this->_items)
      out.push_back(item.weather);
   return out;
}

void RegionWeatherModel::_on_form_deleted(dovah::form_stub& stub) {
   auto&  list = this->_items;
   size_t size = list.size();
   if (stub.form_type == dovah::form_type::sound_descriptor) {
      for (size_t i = 0; i < size; ++i) {
         if (list[i].weather != &stub)
            continue;
         this->beginRemoveRows({}, i, i);
         list.erase(list.begin() + i);
         --i;
         --size;
         this->endRemoveRows();
      }
   } else if (stub.form_type == dovah::form_type::global) {
      for (size_t i = 0; i < size; ++i) {
         if (list[i].global != &stub)
            continue;
         list[i].global = nullptr;
         auto qmi = index(i, Column::GlobalName, {});
         emit dataChanged(qmi, qmi);
      }
   }
}
void RegionWeatherModel::_on_form_modified(dovah::form_stub& stub) {
   QString editor_id;
   auto&   list = this->_items;
   size_t  size = list.size();
   if (stub.form_type == dovah::form_type::sound_descriptor) {
      for (size_t i = 0; i < size; ++i) {
         if (list[i].weather != &stub)
            continue;
         if (editor_id.isEmpty())
            editor_id = QString::fromStdString(stub.editorID);
         list[i].cached.weather_id = editor_id;
         auto qmi = this->index(i, Column::WeatherName, {});
         emit dataChanged(qmi, qmi);
      }
   } else if (stub.form_type == dovah::form_type::global) {
      for (size_t i = 0; i < size; ++i) {
         if (list[i].global != &stub)
            continue;
         if (editor_id.isEmpty())
            editor_id = QString::fromStdString(stub.editorID);
         list[i].cached.global_id = editor_id;
         auto qmi = index(i, Column::GlobalName, {});
         emit dataChanged(qmi, qmi);
      }
   }
}