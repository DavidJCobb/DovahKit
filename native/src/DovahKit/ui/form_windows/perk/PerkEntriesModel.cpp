#include "./PerkEntriesModel.h"
#include "helpers/vectors/re_sort_item_within.h"
#include "dovah/form_stub.h"
#include "editor/core.h"
#include "editor/helpers/actor_value_index_to_name.h"
#include "editor/localize/entry_point_function.h"
#include "editor/localize/perk_entry_point.h"

PerkEntriesModel::PerkEntriesModel(QObject* parent) : QAbstractItemModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent,  this, &PerkEntriesModel::_on_data_abandon_imminent);
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, &PerkEntriesModel::_on_form_deletion_imminent);
   QObject::connect(&editor, &DovahKitCore::formModified, this, &PerkEntriesModel::_on_form_modified);
}
   
#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex PerkEntriesModel::index(int row, int col, const QModelIndex& parent) const /*override*/ {
         if (row < 0 || row >= this->_data.size())
            return {};
         if (col < 0 || col >= ColumnCount)
            return {};
         if (parent.isValid())
            return {};
         return this->createIndex(row, col, nullptr);
      }
      /*virtual*/ QModelIndex PerkEntriesModel::parent(const QModelIndex& index) const /*override*/ {
         return {};
      }
      /*virtual*/ QModelIndex PerkEntriesModel::sibling(int row, int column, const QModelIndex& index) const /*override*/ {
         if (!index.isValid())
            return {};
         return this->index(row, column, {});
      }
      /*virtual*/ int PerkEntriesModel::rowCount(const QModelIndex& parent) const /*override*/ {
         return this->_data.size();
      }
      /*virtual*/ int PerkEntriesModel::columnCount(const QModelIndex& parent) const /*override*/ {
         return ColumnCount;
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant PerkEntriesModel::data(const QModelIndex& index, int role) const /*override*/ {
         if (!index.isValid())
            return {};
         if (index.row() >= this->_data.size())
            return {};
         switch (role) {
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
               break;
            default:
               return {};
         }
         auto& src = this->_data[index.row()];
         switch (index.column()) {
            case Column::Priority:
               return src.data.priority;
            case Column::Rank:
               return src.data.rank;
            case Column::Type:
               if (std::holds_alternative<value_type::quest_entry>(src.data.data)) {
                  return tr("Quest");
               } else if (std::holds_alternative<value_type::spell_entry>(src.data.data)) {
                  return tr("Ability");
               } else if (std::holds_alternative<value_type::entry_point_entry>(src.data.data)) {
                  return tr("Entry Point");
               }
               break;
            case Column::Data1:
               return src.cached_data[0];
            case Column::Data2:
               return src.cached_data[1];
            case Column::Data3:
               return src.cached_data[2];
            case Column::Data4:
               return src.cached_data[3];
         }
         return {};
      }
      /*virtual*/ Qt::ItemFlags PerkEntriesModel::flags(const QModelIndex& index) const /*override*/ {
         if (!index.isValid()) {
            return {};
         }
         auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
         return flags;
      }
   #pragma endregion
      /*virtual*/ QVariant PerkEntriesModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
         if (orientation != Qt::Orientation::Horizontal)
            return {};
         if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
            return {};
         switch (section) {
            case Column::Priority:
               return tr("Priority", "column header");
            case Column::Rank:
               return tr("Rank", "column header");
            case Column::Type:
               return tr("Type", "column header");
            case Column::Data1:
               return tr("Data 1", "column header");
            case Column::Data2:
               return tr("Data 2", "column header");
            case Column::Data3:
               return tr("Data 3", "column header");
            case Column::Data4:
               return tr("Data 4", "column header");
         }
         return {};
      }
#pragma endregion

void PerkEntriesModel::_on_data_abandon_imminent() {
   this->beginResetModel();
   this->_data.clear();
   this->endResetModel();
}
void PerkEntriesModel::_on_form_modified(dovah::form_stub* stub) {
   assert(!!stub);
   auto& list = this->_data;
   size_t size = list.size();
   for (size_t i = 0; i < size; ++i) {
      if (list[i].needs_recache_if_form_changed(*stub)) {
         this->_recache_data(list[i]);
         auto tl = this->index(i, 0, {});
         auto br = this->index(i, ColumnCount, {});
         emit dataChanged(tl, br);
      }
   }
}
void PerkEntriesModel::_on_form_deletion_imminent(dovah::form_stub* stub) {
   assert(!!stub);
   auto&  list = this->_data;
   size_t size = list.size();
   for (size_t i = 0; i < size; ++i) {
      if (list[i].data.sever_references_to(*stub)) {
         this->_recache_data(list[i]);
         auto tl = this->index(i, 0, {});
         auto br = this->index(i, ColumnCount, {});
         emit dataChanged(tl, br);
      }
   }
}

void PerkEntriesModel::initializeFrom(const std::vector<value_type>& src_list) {
   this->beginResetModel();
   const size_t size = src_list.size();
   this->_data.resize(size);
   for (size_t i = 0; i < size; ++i) {
      this->_data[i].data = src_list[i];
      this->_recache_data(this->_data[i]);
   }
   std::sort(
      this->_data.begin(),
      this->_data.end(),
      &PerkEntriesModel::Entry::sort
   );
   this->endResetModel();
}
void PerkEntriesModel::commitTo(std::vector<value_type>& list) const {
   const size_t size = this->_data.size();
   list.resize(size);
   for (size_t i = 0; i < size; ++i)
      list[i] = this->_data[i].data;
}

PerkEntriesModel::value_type PerkEntriesModel::item(size_t row) const {
   if (row >= this->_data.size())
      return {};
   return this->_data[row].data;
}
void PerkEntriesModel::setItem(size_t row, const value_type& src) {
   if (row >= this->_data.size())
      return;
   auto& dst = this->_data[row];
   dst.data = src;
   this->_recache_data(dst);

   auto tl = this->index(row, 0, {});
   auto br = this->index(row, Column::__COUNT - 1, {});
   emit dataChanged(tl, br);

   this->_re_sort_item(dst);
}
void PerkEntriesModel::addItem(const value_type& src) {
   Entry item;
   item.data = src;
   this->_recache_data(item);
   this->_insert_item(item, true);
}
void PerkEntriesModel::deleteItem(size_t row) {
   if (row >= this->_data.size())
      return;
   this->beginRemoveRows({}, row, row);
   this->_data.erase(this->_data.begin() + row);
   this->endRemoveRows();
}

void PerkEntriesModel::_insert_item(const Entry& item, bool emit_model_sync_signals) {
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
decltype(PerkEntriesModel::_data)::iterator PerkEntriesModel::_insertion_point_for(const Entry& item) {
   return std::upper_bound(
      this->_data.begin(),
      this->_data.end(),
      item,
      &PerkEntriesModel::Entry::sort
   );
}
void PerkEntriesModel::_re_sort_item(const Entry& item) {
   auto& list = this->_data;
   auto entry_it = std::find(list.begin(), list.end(), item);
   if (entry_it == list.end())
      return;
   size_t from = std::distance(list.begin(), entry_it);
   this->_re_sort_item(from);
}
void PerkEntriesModel::_re_sort_item(size_t from) {
   auto& list = this->_data;
   if (from >= list.size())
      return;
   
   bool moved = false;
   cobb::vectors::re_sort_item_within(
      list,
      list.begin() + from,
      &PerkEntriesModel::Entry::sort,
      [&moved, this, &list](decltype(_data)::iterator from_it, decltype(_data)::iterator to_it) {
         moved = true;
         size_t from  = std::distance(list.begin(), from_it);
         size_t to    = std::distance(list.begin(), to_it);

         this->beginMoveRows(
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

void PerkEntriesModel::_recache_data(Entry& item) {
   auto& src = item.data.data;
   auto& dst = item.cached_data;
   if (auto* casted_ptr = std::get_if<value_type::quest_entry>(&src)) {
      if (auto* stub = casted_ptr->quest) {
         dst[0] = QString::fromStdString(stub->get_editor_id());
      } else {
         dst[0] = tr("NONE", "form editor ID");
      }
      dst[1] = QString::number(casted_ptr->stage);
      dst[2].clear();
      dst[3].clear();
   } else if (auto* casted_ptr = std::get_if<value_type::spell_entry>(&src)) {
      if (auto* stub = casted_ptr->spell) {
         dst[0] = QString::fromStdString(stub->get_editor_id());
      } else {
         dst[0] = tr("NONE", "form editor ID");
      }
      for (size_t i = 1; i < dst.size(); ++i)
         dst[i].clear();
   } else if (auto* casted_ptr = std::get_if<value_type::entry_point_entry>(&src)) {
      dst[0] = editor::localize::perk_entry_point(casted_ptr->entry_point);
      dst[1] = editor::localize::entry_point_function(casted_ptr->function);
      dst[2].clear();
      dst[3].clear();
      
      auto& params = casted_ptr->parameters;
      if (auto* casted_params = std::get_if<ui::types::perk_entries::params::activate_choice>(&params)) {
         dst[2] = casted_params->label;
         if (auto* stub = casted_params->spell) {
            dst[3] = QString::fromStdString(stub->get_editor_id());
         } else {
            dst[3] = tr("NONE", "form editor ID");
         }
      } else if (auto* casted_params = std::get_if<ui::types::perk_entries::params::form>(&params)) {
         if (auto* stub = casted_params->value) {
            dst[2] = QString::fromStdString(stub->get_editor_id());
         } else {
            dst[2] = tr("NONE", "form editor ID");
         }
      } else if (auto* casted_params = std::get_if<ui::types::perk_entries::params::localized_string>(&params)) {
         dst[2] = casted_params->value;
      } else if (auto* casted_params = std::get_if<ui::types::perk_entries::params::one_av_one_float>(&params)) {
         dst[2] = editor_helpers::actor_value_index_to_name(casted_params->actor_value);
         dst[3] = QString::number(casted_params->value);
      } else if (auto* casted_params = std::get_if<ui::types::perk_entries::params::one_float>(&params)) {
         dst[2] = QString::number(casted_params->value);
      } else if (auto* casted_params = std::get_if<ui::types::perk_entries::params::raw_string>(&params)) {
         dst[2] = casted_params->value;
      } else if (auto* casted_params = std::get_if<ui::types::perk_entries::params::two_floats>(&params)) {
         dst[2] = QString::number(casted_params->values[0]);
         dst[3] = QString::number(casted_params->values[1]);
      }
   }
}

bool PerkEntriesModel::Entry::needs_recache_if_form_changed(const dovah::form_stub& stub) const {
   auto& src = this->data.data;
   if (auto* casted_ptr = std::get_if<value_type::quest_entry>(&src)) {
      return casted_ptr->quest == &stub;
   } else if (auto* casted_ptr = std::get_if<value_type::spell_entry>(&src)) {
      return casted_ptr->spell == &stub;
   } else if (auto* casted_ptr = std::get_if<value_type::entry_point_entry>(&src)) {
      auto& params = casted_ptr->parameters;
      if (auto* casted_params = std::get_if<ui::types::perk_entries::params::activate_choice>(&params)) {
         return casted_params->spell == &stub;
      } else if (auto* casted_params = std::get_if<ui::types::perk_entries::params::form>(&params)) {
         return casted_params->value == &stub;
      }
   }
   return false;
}
/*static*/ bool PerkEntriesModel::Entry::sort(const Entry& a, const Entry& b) {
   if (a.data.rank == b.data.rank)
      return a.data.priority < b.data.priority;
   return a.data.rank < b.data.rank;
}