#include "./RaceEquipSlotsModel.h"
#include "dovah/forms/EquipSlot.h"
#include "dovah/form_stub.h"
#include "dovah/form_reference_t.h"
#include "editor/core.h"

RaceEquipSlotsModel::RaceEquipSlotsModel(QObject* parent) : QAbstractItemModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAcquireComplete,  this, &RaceEquipSlotsModel::_on_data_acquire);
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent,  this, &RaceEquipSlotsModel::_on_data_abandon_imminent);
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, &RaceEquipSlotsModel::_on_form_deletion_imminent);
   QObject::connect(&editor, &DovahKitCore::formCreated,  this, &RaceEquipSlotsModel::_on_form_created);
   QObject::connect(&editor, &DovahKitCore::formModified, this, &RaceEquipSlotsModel::_on_form_modified);
   if (editor.has_data()) {
      this->_on_data_acquire();
   }
}
   
#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex RaceEquipSlotsModel::index(int row, int col, const QModelIndex& parent) const /*override*/ {
         if (row < 0 || row >= this->_data.size())
            return {};
         if (col < 0 || col >= ColumnCount)
            return {};
         if (parent.isValid())
            return {};
         return this->createIndex(row, col, nullptr);
      }
      /*virtual*/ QModelIndex RaceEquipSlotsModel::parent(const QModelIndex& index) const /*override*/ {
         return {};
      }
      /*virtual*/ QModelIndex RaceEquipSlotsModel::sibling(int row, int column, const QModelIndex& index) const /*override*/ {
         if (!index.isValid())
            return {};
         return this->index(row, column, {});
      }
      /*virtual*/ int RaceEquipSlotsModel::rowCount(const QModelIndex& parent) const /*override*/ {
         return this->_data.size();
      }
      /*virtual*/ int RaceEquipSlotsModel::columnCount(const QModelIndex& parent) const /*override*/ {
         return ColumnCount;
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant RaceEquipSlotsModel::data(const QModelIndex& index, int role) const /*override*/ {
         if (!index.isValid())
            return {};
         if (index.row() >= this->_data.size())
            return {};
         auto& src = this->_data[index.row()];
         switch (role) {
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
               return src.cached_editor_id;
            case Qt::CheckStateRole:
               return src.checked ? Qt::CheckState::Checked : Qt::CheckState::Unchecked;
         }
         return {};
      }
      /*virtual*/ Qt::ItemFlags RaceEquipSlotsModel::flags(const QModelIndex& index) const /*override*/ {
         if (!index.isValid()) {
            return {};
         }
         auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
         flags |= Qt::ItemFlag::ItemIsUserCheckable;
         return flags;
      }
      #pragma region Write-access
         /*virtual*/ bool RaceEquipSlotsModel::setData(const QModelIndex& index, const QVariant& value, int role) /*override*/ {
            if (!index.isValid() || index.row() >= this->_data.size() || index.column() >= ColumnCount)
               return false;
            if (role != Qt::CheckStateRole)
               return false;

            this->_data[index.row()].checked = (value.toInt() == Qt::CheckState::Checked) ? true : false;
            emit dataChanged(index, index, { role });
            return true;
         }
      #pragma endregion
   #pragma endregion
      /*virtual*/ QVariant RaceEquipSlotsModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
         if (orientation != Qt::Orientation::Horizontal)
            return {};
         if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
            return {};
         return tr("Equip Slots");
      }
#pragma endregion

void RaceEquipSlotsModel::_on_data_acquire() {
   this->beginResetModel();
   this->_data.clear();
   DovahKitCore::get().for_each_form_of_type(dovah::form_type::equip_slot, [this](dovah::form_stub* stub) {
      KnownForm item;
      item.stub    = stub;
      item.cached_editor_id = QString::fromStdString(stub->editorID);
      item.checked = false;
      this->_insert_item(item, true);
      return false;
   });
   this->endResetModel();
}
void RaceEquipSlotsModel::_on_data_abandon_imminent() {
   this->beginResetModel();
   this->_data.clear();
   this->endResetModel();
}
void RaceEquipSlotsModel::_on_form_created(dovah::form_stub* stub) {
   if (stub->form_type != dovah::form_type::equip_slot)
      return;
   KnownForm item;
   item.stub    = stub;
   item.cached_editor_id = QString::fromStdString(stub->editorID);
   item.checked = false;
   this->_insert_item(item, true);
}
void RaceEquipSlotsModel::_on_form_modified(dovah::form_stub* stub) {
   if (stub->form_type != dovah::form_type::equip_slot)
      return;

   QString editor_id_prior;
   QString editor_id_after = QString::fromStdString(stub->editorID);

   auto&  list = this->_data;
   size_t from;
   for (from = 0; from < list.size(); ++from) {
      auto& item = list[from];
      if (item.stub == stub) {
         break;
      }
   }
   bool use_all_parents = false;
   {
      auto loaded = stub->load().ptr_cast<dovah::loaded_forms::EquipSlot>();
      if (loaded)
         use_all_parents = loaded->local_flags & dovah::loaded_forms::EquipSlot::local_flag::use_all_parents;
   }

   if (from >= list.size()) {
      //
      // This form isn't in our list.
      //
      if (!use_all_parents) {
         KnownForm item;
         item.stub    = stub;
         item.cached_editor_id = QString::fromStdString(stub->editorID);
         item.checked = false;
         this->_insert_item(item, true);
      }
      return;
   }
   //
   // This form is in our list. Update it.
   //
   if (use_all_parents) {
      this->beginRemoveRows({}, from, from);
      list.erase(list.begin() + from);
      this->endRemoveRows();
      return;
   }
   //
   // If the editor ID changed, re-sort the list item.
   //
   auto& item = list[from];
   editor_id_prior = item.cached_editor_id;
   if (editor_id_prior != editor_id_after) {
      item.cached_editor_id = editor_id_after;
      this->_re_sort_item(item, editor_id_prior);
   }
}
void RaceEquipSlotsModel::_on_form_deletion_imminent(dovah::form_stub* stub) {
   if (stub->form_type != dovah::form_type::equip_slot)
      return;
   auto&  list = this->_data;
   size_t size = list.size();
   for (size_t i = 0; i < size; ++i) {
      if (list[i].stub == stub) {
         this->beginRemoveRows({}, i, i);
         list.erase(list.begin() + i);
         this->endRemoveRows();
      }
   }
}

void RaceEquipSlotsModel::initializeFrom(const std::vector<dovah::form_reference_t>& src_list) {
   this->beginResetModel();
   for (auto& item : this->_data) {
      item.checked = false;
   }
   for (auto& form_use : src_list) {
      for (auto& item : this->_data) {
         if (item.stub == form_use.get_form_stub()) {
            item.checked = true;
            break;
         }
      }
   }
   this->endResetModel();
}
void RaceEquipSlotsModel::commitTo(std::vector<dovah::form_reference_t>& list, dovah::loaded_forms::Form& containing_form) const {
   for (auto& item : list)
      item.set(containing_form, nullptr);
   list.clear();

   for (auto& item : this->_data) {
      if (!item.checked)
         continue;
      auto& dst = list.emplace_back();
      dst.set(containing_form, item.stub);
   }
}

bool RaceEquipSlotsModel::isChecked(size_t row) const {
   if (row >= this->_data.size())
      return false;
   return this->_data[row].checked;
}
bool RaceEquipSlotsModel::isChecked(const dovah::form_stub& stub) const {
   for (size_t i = 0; i < this->_data.size(); ++i)
      if (this->_data[i].stub == &stub)
         return this->_data[i].checked;
   return false;
}
void RaceEquipSlotsModel::setChecked(size_t row, bool checked) {
   if (row >= this->_data.size())
      return;
   auto& dst = this->_data[row].checked;
   if (dst == checked)
      return;
   dst = checked;

   auto qmi = this->index(row, 0, {});
   emit dataChanged(qmi, qmi, { Qt::CheckStateRole });
}
void RaceEquipSlotsModel::setChecked(const dovah::form_stub& stub, bool checked) {
   for (size_t i = 0; i < this->_data.size(); ++i) {
      auto& item = this->_data[i];
      if (item.stub != &stub)
         continue;
      if (item.checked == checked)
         return;
      item.checked = checked;

      auto qmi = this->index(i, 0, {});
      emit dataChanged(qmi, qmi, { Qt::CheckStateRole });
      return;
   }
}

void RaceEquipSlotsModel::_insert_item(const KnownForm& item, bool emit_model_sync_signals) {
   auto dst_it = this->_insertion_point_for(item);
   if (emit_model_sync_signals) {
      auto index = std::distance(this->_data.begin(), dst_it);
      this->beginInsertRows({}, index, index);
   }
   this->_data.insert(dst_it, item);
   if (emit_model_sync_signals) {
      this->endInsertRows();
   }
}
decltype(RaceEquipSlotsModel::_data)::iterator RaceEquipSlotsModel::_insertion_point_for(const KnownForm& item) {
   if (!item.stub)
      //
      // A "NONE" entry should always be prepended to the top of the sorted list.
      //
      return this->_data.begin();

   return std::upper_bound(
      this->_data.begin(),
      this->_data.end(),
      item,
      [](const KnownForm& a, const KnownForm& b) -> bool {
         if (!a.stub && b.stub)
            return true;
         if (!b.stub && a.stub)
            return false;
         return a.cached_editor_id.compare(b.cached_editor_id, Qt::CaseInsensitive) < 0;
      }
   );
}
void RaceEquipSlotsModel::_re_sort_item(const KnownForm& item, std::optional<QString> prior_name) {
   auto& list = this->_data;
            
   auto entry_it = std::find(list.begin(), list.end(), item);
   if (entry_it == list.end())
      return;
   size_t from = std::distance(list.begin(), entry_it);
   size_t to;
   bool   moving_upward_in_list;
   {
      auto dst_it = this->_insertion_point_for(item);
      //
      // Can't use the iterator directly because we'll be doing a removal first, which will 
      // invalidate it.
      //
      to = std::distance(list.begin(), dst_it);
      moving_upward_in_list = dst_it < entry_it;
   }
   this->beginMoveRows(
      {},
      from, // first to move
      from, // last  to move
      {},
      to
   );
   if (!moving_upward_in_list) {
      //
      // We move `entry` by first removing it from the list, and then inserting it into the 
      // list at the desired index. If we're moving `entry` downward within the list, then 
      // its removal will displace the intended destination by -1.
      //
      --to;
   }
   list.erase(entry_it);
   list.insert(list.begin() + to, item);
   this->endMoveRows();
}