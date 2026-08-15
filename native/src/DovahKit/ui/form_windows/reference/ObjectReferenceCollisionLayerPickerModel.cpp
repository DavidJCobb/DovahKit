#include "./ObjectReferenceCollisionLayerPickerModel.h"
#include "helpers/vectors/re_sort_item_within.h"
#include "dovah/data/collision_layers.h"
#include "editor/core.h"
#include "editor/subsystems/form_info_cache/cached_data/by_form_type/collision_layer.h"
#include "editor/subsystems/form_info_cache/core.h"
#include "editor/form_stub_meta_type.h"
#include "editor/localize/collision_layer.h"

ObjectReferenceCollisionLayerPickerModel::ObjectReferenceCollisionLayerPickerModel(QObject* parent) : QAbstractItemModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent,  this, &ObjectReferenceCollisionLayerPickerModel::_on_data_abandoned);
   QObject::connect(&editor, &DovahKitCore::formCreated,          this, [this](dovah::form_stub* stub) { this->_insert_form_if_valid(*stub, true); });
   QObject::connect(&editor, &DovahKitCore::formModified,         this, [this](dovah::form_stub* stub) { this->_on_form_modified(*stub); });
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub) { this->_on_form_deleted(*stub); });
   if (editor.has_data())
      _gather_all_layers();
}
   
#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex ObjectReferenceCollisionLayerPickerModel::index(int row, int col, const QModelIndex& parent) const /*override*/ {
         if (row < 0 || row >= this->_data.size())
            return {};
         if (col < 0 || col >= ColumnCount)
            return {};
         if (parent.isValid())
            return {};
         return this->createIndex(row, col, nullptr);
      }
      /*virtual*/ QModelIndex ObjectReferenceCollisionLayerPickerModel::parent(const QModelIndex& index) const /*override*/ {
         return {};
      }
      /*virtual*/ QModelIndex ObjectReferenceCollisionLayerPickerModel::sibling(int row, int column, const QModelIndex& index) const /*override*/ {
         if (!index.isValid())
            return {};
         return this->index(row, column, {});
      }
      /*virtual*/ int ObjectReferenceCollisionLayerPickerModel::rowCount(const QModelIndex& parent) const /*override*/ {
         return this->_data.size();
      }
      /*virtual*/ int ObjectReferenceCollisionLayerPickerModel::columnCount(const QModelIndex& parent) const /*override*/ {
         return ColumnCount;
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant ObjectReferenceCollisionLayerPickerModel::data(const QModelIndex& index, int role) const /*override*/ {
         if (!index.isValid())
            return {};
         if (index.row() >= this->_data.size())
            return {};
         auto& src = this->_data[index.row()];
         switch (role) {
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
               return src.name;
            case Qt::UserRole:
               return src.layer_uid;
         }
         return {};
      }
      /*virtual*/ Qt::ItemFlags ObjectReferenceCollisionLayerPickerModel::flags(const QModelIndex& index) const /*override*/ {
         if (!index.isValid()) {
            return {};
         }
         auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
         return flags;
      }
   #pragma endregion
   /*virtual*/ QVariant ObjectReferenceCollisionLayerPickerModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
      return {};
   }
#pragma endregion

void ObjectReferenceCollisionLayerPickerModel::setIsForTriggerVolume(bool v) {
   if (v == this->_is_for_trigger_volume)
      return;
   this->_is_for_trigger_volume = v;
   this->_gather_all_layers();
}

bool ObjectReferenceCollisionLayerPickerModel::_is_valid(const dovahkit::subsystems::form_info_cache::cached_data::by_form::collision_layer& info) {
   if ((dovah::collision_layer)info.unique_id == dovah::collision_layer::unidentified)
      return true;
   if (this->_is_for_trigger_volume)
      if (!info.flags.trigger)
         return false;
   if (info.unique_id >= dovah::all_collision_layers.size())
      return false;
   return true;
}
size_t ObjectReferenceCollisionLayerPickerModel::_index_of(const dovah::form_stub& stub) const {
   if (stub.form_type != dovah::form_type::collision_layer)
      return (size_t)-1;
   for (size_t i = 0; i < this->_data.size(); ++i)
      if (this->_data[i].form == &stub)
         return i;
   return (size_t)-1;
}

/*static*/ bool ObjectReferenceCollisionLayerPickerModel::_compare_for_sort(const Item& a, const Item& b) {
   return a.name.compare(b.name, Qt::CaseInsensitive) < 0;
}
int ObjectReferenceCollisionLayerPickerModel::_insert_item(const Item& item, bool emit_model_sync_signals) {
   auto dst_it = this->_insertion_point_for(item);
   auto row   = std::distance(this->_data.begin(), dst_it);
   if (emit_model_sync_signals) {
      this->beginInsertRows({}, row, row);
   }
   this->_data.insert(dst_it, item);
   if (emit_model_sync_signals) {
      this->endInsertRows();
   }
   return row;
}
decltype(ObjectReferenceCollisionLayerPickerModel::_data)::iterator ObjectReferenceCollisionLayerPickerModel::_insertion_point_for(const Item& item) {
   return std::upper_bound(
      this->_data.begin(),
      this->_data.end(),
      item,
      &_compare_for_sort
   );
}
void ObjectReferenceCollisionLayerPickerModel::_re_sort_item(size_t from) {
   auto& list = this->_data;
   if (from >= list.size())
      return;
   
   bool moved = false;
   cobb::vectors::re_sort_item_within(
      list,
      list.begin() + from,
      &_compare_for_sort,
      [&moved, this, &list](decltype(_data)::iterator from_it, decltype(_data)::iterator to_it) {
         size_t from  = std::distance(list.begin(), from_it);
         size_t to    = std::distance(list.begin(), to_it);
         moved = this->beginMoveRows(
            {},
            from, // first to move
            from, // last  to move
            {},
            (to < from) ? to : to + 1 // Qt API design jank
         );
      }
   );
   if (moved)
      this->endMoveRows();
}

void ObjectReferenceCollisionLayerPickerModel::_gather_all_layers() {
   this->beginResetModel();
   this->_data.clear();
   auto& editor = DovahKitCore::get();
   editor.for_each_form_of_type(dovah::form_type::collision_layer, [this](dovah::form_stub* stub) -> bool {
      this->_insert_form_if_valid(*stub, false);
      return false;
   });
   this->endResetModel();
}
void ObjectReferenceCollisionLayerPickerModel::_on_data_abandoned() {
   this->beginResetModel();
   this->_data.clear();
   this->endResetModel();
}
void ObjectReferenceCollisionLayerPickerModel::_on_form_deleted(dovah::form_stub& stub) {
   auto i = _index_of(stub);
   if (i == (size_t)-1)
      return;
   this->beginRemoveRows({}, i, i);
   this->_data.erase(this->_data.begin() + i);
   --i;
   this->endRemoveRows();
}
void ObjectReferenceCollisionLayerPickerModel::_on_form_modified(dovah::form_stub& stub) {
   auto& fic  = dovahkit::subsystems::form_info_cache::core::get();
   auto* info = fic.get_collision_layer_info(stub);

   auto i = _index_of(stub);
   if (i == (size_t)-1) {
      if (!info)
         return;
      //
      // Layer was previously unknown or invalid.
      //
      this->_insert_form_if_valid(stub, true);
   } else {
      if (info && _is_valid(*info)) {
         auto& item = this->_data[i];
         item.layer_uid = info->unique_id;
         item.name      = editor::localize::collision_layer((dovah::collision_layer)info->unique_id);

         auto qmi = this->index(i, 0, {});
         emit dataChanged(qmi, qmi);
         this->_re_sort_item(i);
      } else {
         //
         // Layer is no longer valid for use here.
         //
         this->beginRemoveRows({}, i, i);
         this->_data.erase(this->_data.begin() + i);
         --i;
         this->endRemoveRows();
      }
   }
}

void ObjectReferenceCollisionLayerPickerModel::_insert_form_if_valid(dovah::form_stub& stub, bool emit_signals) {
   if (stub.form_type != dovah::form_type::collision_layer)
      return;

   auto& fic  = dovahkit::subsystems::form_info_cache::core::get();
   auto* info = fic.get_collision_layer_info(stub);
   if (!info || !_is_valid(*info))
      return;

   auto item = Item{
      .form      = &stub,
      .layer_uid = info->unique_id,
      .name      = editor::localize::collision_layer((dovah::collision_layer)info->unique_id),
   };
   auto it = _insertion_point_for(item);
   if (emit_signals) {
      auto i = std::distance(this->_data.begin(), it);
      this->beginInsertRows({}, i, i);
   }
   this->_data.insert(it, std::move(item));
   if (emit_signals) {
      this->endInsertRows();
   }
}