#include "./DKFormPickerDialogModel.h"
#include <cassert>
#include "helpers/qt/strings.h"
#include "editor/core.h"
#include "editor/form_stub_meta_type.h"
#include "editor/helpers/form_identifiers_to_string.h"

namespace ui::impl::DKFormPicker {
   DialogModel::DialogModel(QObject* parent) {
      auto& source = shared_datastore::get();
      QObject::connect(&source, &shared_datastore::rowsInserted, this, [this](size_t first, size_t last) {
         auto& source = shared_datastore::get();
         for (size_t i = first; i <= last; ++i) {
            auto* entry = source.item_at_row(i);
            if (!this->_entry_matches_params(*entry))
               continue;

            auto it  = this->_insertion_point_for(*entry);
            int  pos = it - this->_items.begin();
            this->beginInsertRows({}, pos, pos);
            this->_items.insert(it, entry);
            this->endInsertRows();
         }
      });
      QObject::connect(&source, &shared_datastore::rowsAboutToBeRemoved, this, [this](size_t first, size_t last) {
         if (this->_items.empty()) // this check, in conjunction with the allDataCleared signal, means we don't need to do any advanced processing for unloading all data
            return;
         auto& source = shared_datastore::get();
         //
         // Prune the sorted list.
         //
         for (size_t i = 0; i <= last; ++i) {
            auto* entry = source.item_at_row(i);
            assert(entry != nullptr);
            auto it = std::find(this->_items.begin(), this->_items.end(), entry);
            if (it == this->_items.end())
               continue;
            {
               auto index = std::distance(this->_items.begin(), it);
               this->beginRemoveRows({}, index, index);
            }
            this->_items.erase(it);
            this->endRemoveRows();
         }
      });
      QObject::connect(&source, &shared_datastore::allDataCleared, this, [this]() {
         this->_items.clear();
      });
      QObject::connect(&source, &shared_datastore::editorIDChanged, this, [this](const item_type& item, const QString& prior) {
         this->_re_sort_item(item, prior);
      });
      QObject::connect(&source, &shared_datastore::itemExclusionStateChanged, this, [this](const item_type& entry, bool exclude_now) {
         this->_on_item_exclusion_state_changed(entry, exclude_now);
      });
   }

   #pragma region QAbstractItemModel boilerplate
      #pragma region Hierarchy
         /*virtual*/ QModelIndex DialogModel::index(int row, int column, const QModelIndex& parent) const /*override*/ {
            if (!this->hasIndex(row, column, parent))
               return {};
            auto size = this->_items.size();
            if (row < 0 || row >= size)
               return {};
            return this->createIndex(row, column, const_cast<item_type*>(this->_items[row]));
         }
         /*virtual*/ QModelIndex DialogModel::parent(const QModelIndex& index) const /*override*/ {
            return {};
         }
         /*virtual*/ int DialogModel::rowCount(const QModelIndex& parent) const /*override*/ {
            return this->_items.size();
         }
         /*virtual*/ int DialogModel::columnCount(const QModelIndex& item) const /*override*/ {
            return 3;
         }
      #pragma endregion
      #pragma region Data
         /*virtual*/ QVariant DialogModel::data(const QModelIndex& index, int role) const /*override*/ {
            if (!index.isValid())
               return {};
            int i = index.row();
            if (i < 0 || i >= this->_items.size())
               return {};
            const auto* entry = this->_items[i];

            if (role == FormStubRole)
               return QVariant::fromValue(entry->stub);

            if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
               switch (index.column()) {
                  case 0:
                     if (!entry->stub) {
                        return tr("NONE");
                     }
                     if (entry->editorID.isEmpty()) {
                        return tr("<Unnamed: %1>").arg(QString("%1").arg(entry->stub->formID, 8, 16, QChar('0')).toUpper());
                     }
                     return entry->editorID;
                  case 1:
                     if (!entry->stub)
                        return {};
                     return cobb::qt::four_cc_to_string(dovah::form_type_info::lookup(entry->stub->form_type).signature);
                  case 2:
                     if (!entry->stub)
                        return {};
                     return editor_helpers::form_id_to_string(entry->stub->formID);
               }
               return {};
            }
            return {};
         }
         /*virtual*/ Qt::ItemFlags DialogModel::flags(const QModelIndex& index) const /*override*/ {
            if (!index.isValid())
               return Qt::NoItemFlags;
            return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled;
         }
         /*virtual*/ QVariant DialogModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
            if (orientation != Qt::Orientation::Horizontal)
               return {};
            if (role == Qt::DisplayRole) {
               switch (section) {
                  case 0:
                     return tr("Form");
                  case 1:
                     return tr("Type");
                  case 2:
                     return tr("ID");
               }
               return {};
            }
            return {};
         }
      #pragma endregion
   #pragma endregion

   bool DialogModel::_entry_matches_params(const item_type& item) const {
      if (!this->_allowed_form_types.isEmpty()) {
         if (!this->_allowed_form_types.contains(item.type))
            return false;
      }
      if (!item.stub)
         return false;
      if (!_entry_matches_filter_string(item))
         return false;
      return true;
   }
   bool DialogModel::_entry_matches_filter_string(const item_type& item) const {
      QString item_name = item.editorID;
      if (item_name.isEmpty()) {
         if (item.stub == nullptr)
            item_name = tr("NONE");
         else
            item_name = tr("<Unnamed: %1>").arg(QString("%1").arg(item.stub->formID, 8, 16, QChar('0')).toUpper());
      }
      return item_name.contains(this->_filter, Qt::CaseInsensitive);
   }

   decltype(DialogModel::_items)::iterator DialogModel::_insertion_point_for(const item_type& item) {
      if (!item.stub)
         //
         // A "NONE" entry should always be prepended to the top of the sorted list.
         //
         return this->_items.begin();

      return std::upper_bound(
         this->_items.begin(),
         this->_items.end(),
         &item,
         [](const item_type* a, const item_type* b) {
            if (!a->stub && b->stub)
               return true;
            if (!b->stub && a->stub)
               return false;
            return a->editorID.compare(b->editorID, Qt::CaseInsensitive) < 0;
         }
      );
   }

   void DialogModel::_refill() {
      this->beginResetModel();

      this->_items.clear();

      std::vector<const item_type*> unsorted;
      {
         auto&  source = shared_datastore::get();
         size_t max    = source.size();
         for (size_t i = 0; i < max; ++i) {
            auto* entry = source.item_at_row(i);
            if (!entry)
               continue;
            auto* stub = entry->stub;
            if (!stub)
               continue;
            if (!this->_entry_matches_params(*entry))
               continue;
            if (entry->default_exclude_from_listings)
               continue;
            unsorted.push_back(entry);
         }
      }
      for (auto* entry : unsorted) {
         this->_items.insert(this->_insertion_point_for(*entry), entry);
      }

      this->endResetModel();
   }
   void DialogModel::_tighten_filter() {
      size_t size = this->_items.size();
      for (size_t i = 0; i < size; ++i) {
         auto* entry = this->_items[i];
         assert(entry != nullptr);
         if (this->_entry_matches_filter_string(*entry))
            continue;

         this->beginRemoveRows({}, i, i);
         this->_items.erase(this->_items.begin() + i);
         this->endRemoveRows();
         --i;
         --size;
      }
   }

   void DialogModel::_re_sort_item(const item_type& item, std::optional<QString> prior_name) {
      auto& list = this->_items;
            
      auto entry_it = std::find(list.begin(), list.end(), &item);
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
      list.insert(list.begin() + to, &item);
      this->endMoveRows();
   }

   void DialogModel::_on_item_exclusion_state_changed(const item_type& entry, bool exclude_now) {
      if (!this->_entry_matches_params(entry))
         return;

      if (exclude_now) {
         this->_force_remove_item(entry);
      } else {
         this->_force_insert_item(entry);
      }
   }

   void DialogModel::_force_insert_item(const item_type& item, bool emit_model_sync_signals) {
      if (this->_contains_item(item))
         return;
            
      auto dst_it = this->_insertion_point_for(item);
      if (emit_model_sync_signals) {
         auto index = std::distance(this->_items.begin(), dst_it);
         this->beginInsertRows({}, index, index);
      }
      this->_items.insert(dst_it, &item);
      if (emit_model_sync_signals) {
         this->endInsertRows();
      }
   }
   void DialogModel::_force_remove_item(const item_type& item, bool emit_model_sync_signals) {
      auto it = std::find(this->_items.begin(), this->_items.end(), &item);
      if (it == this->_items.end())
         return;

      if (emit_model_sync_signals) {
         auto index = std::distance(this->_items.begin(), it);
         this->beginRemoveRows({}, index, index);
      }
      this->_items.erase(it);
      if (emit_model_sync_signals) {
         this->endRemoveRows();
      }
   }

   void DialogModel::setAllowedFormTypes(const QList<dovah::form_type>& desired) {
      if (this->_allowed_form_types == desired)
         return;

      this->_allowed_form_types = desired;
      if (!this->updatesEnabled()) {
         this->_updates_pending = true;
         return;
      }
      this->_refill();
   }

   void DialogModel::setFilterString(QString desired) {
      auto prior = this->_filter;
      if (prior.compare(desired, Qt::CaseInsensitive) == 0)
         return;
      this->_filter = desired;
      if (!this->updatesEnabled()) {
         this->_updates_pending = true;
         return;
      }
      if (prior.startsWith(desired) || prior.endsWith(desired)) {
         this->_tighten_filter();
      } else {
         this->_refill();
      }
   }

   void DialogModel::setUpdatesEnabled(bool b) {
      if (b == this->updatesEnabled())
         return;
      this->_updates_enabled = b;
      if (b) {
         if (this->_updates_pending) {
            this->_updates_pending = false;
            this->_refill();
         }
      }
   }

   QModelIndex DialogModel::index(const dovah::form_stub* stub) const {
      for (size_t i = 0; i < this->_items.size(); ++i)
         if (this->_items[i]->stub == stub)
            return this->index(i, 0, {});
      return {};
   }
}