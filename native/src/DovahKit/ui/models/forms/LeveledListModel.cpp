#include "./LeveledListModel.h"
#include <limits>
#include "dovah/forms/components/leveled_list.h"
#include "dovah/form_stub.h"
#include "editor/core.h"

namespace {
   static const QVector<int> typical_roles_to_notify_changes_for = { Qt::DisplayRole, Qt::ToolTipRole };
}

LeveledListModel::LeveledListModel(QObject* parent) : QAbstractItemModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() {
      this->_clear();
   });
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub, bool just_being_flagged) {
      auto&  list = this->_items;
      size_t size = list.size();
      for (size_t i = 0; i < size; ++i) {
         auto& item = *list[i];
         if (item.form == stub) {
            this->beginRemoveRows({}, i, i);
            delete list[i];
            list.erase(list.begin() + i);
            --i;
            --size;
            this->endRemoveRows();
            continue;
         }
         if (item.ownership.owner == stub) {
            item.ownership.owner = nullptr;
            item.cached.ownerEditorID.clear();
            
            auto qmi = this->index(i, Column::Owner, {});
            emit dataChanged(qmi, qmi, typical_roles_to_notify_changes_for);
         }
         if (item.ownership.global == stub) {
            item.ownership.global = nullptr;
         }
      }
   });
   QObject::connect(&editor, &DovahKitCore::formModified, this, [this](const dovah::form_stub* stub) {
      // Variables for dataChanged signal
      size_t start;
      size_t end;
      auto _update_column_range = [&start, &end](int col) {
         if (start > col)
            start = col;
         if (end < col)
            end = col;
      };

      for (size_t i = 0; i < this->_items.size(); ++i) {
         auto& item = *this->_items[i];
         if (stub == item.form || stub == item.ownership.owner) {
            start = std::numeric_limits<size_t>::max();
            end   = 0;

            if (stub == item.form) {
               item.cached.editorID = QString::fromStdString(stub->editorID);
               _update_column_range(Column::Form);
            }
            if (item.ownership.owner == stub) {
               item.cached.ownerEditorID = QString::fromStdString(stub->editorID);
               _update_column_range(Column::Owner);
            }

            QModelIndex tl = this->index(i, start, {});
            QModelIndex br = this->index(i, end,   {});
            emit dataChanged(tl, br, typical_roles_to_notify_changes_for);
         }
      }
   });
}
LeveledListModel::~LeveledListModel() {
   this->_clear();
}
      
#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex LeveledListModel::index(int row, int column, const QModelIndex& parent) const /*override*/ {
         if (row < 0 || column < 0)
            return {};
         if (column >= this->columnCount())
            return {};
         if (row >= this->rowCount())
            return {};
         if (parent.isValid()) // no nesting
            return {};
         return this->createIndex(row, column, (void*)this->_items[row]);
      }
      /*virtual*/ QModelIndex LeveledListModel::parent(const QModelIndex& index) const /*override*/ {
         return {};
      }
      /*virtual*/ QModelIndex LeveledListModel::sibling(int row, int column, const QModelIndex& index) const /*override*/ {
         if (row < 0 || column < 0)
            return {};
         if (column >= this->columnCount())
            return {};
         if (row >= this->rowCount())
            return {};
         if (!index.isValid() || !index.internalPointer())
            return {};
         return this->createIndex(row, column, (void*)this->_items[row]);
      }
      /*virtual*/ int LeveledListModel::rowCount(const QModelIndex& parent) const /*override final*/ {
         return this->_items.size();
      }
      /*virtual*/ int LeveledListModel::columnCount(const QModelIndex& item) const /*override final*/ {
         if (this->showsContainerItemFields())
            return Column::_COUNT_IF_ITEMS;
         return Column::_COUNT_ALWAYS;
      }

      /*virtual*/ QVariant LeveledListModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
         if (orientation != Qt::Orientation::Horizontal)
            return {};
         if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
            return {};
         switch (section) {
            case Column::Level:
               return tr("Level", "column header");
            case Column::Count:
               return tr("Count", "column header");
            case Column::Form:
               return tr("Form Editor ID", "column header");
            case Column::Owner:
               return tr("Owner", "column header");
            case Column::Health:
               return tr("Health", "column header");
         }
         return {};
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant LeveledListModel::data(const QModelIndex& index, int role) const /*override*/ {
         if (!index.isValid())
            return {};
         auto item   = (LeveledObject*)index.internalPointer();
         auto column = index.column();
         switch (role) {
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
               switch (column) {
                  case Column::Level:
                     return item->level;
                  case Column::Count:
                     return item->count;
                  case Column::Form:
                     return item->cached.editorID;
                  case Column::Owner:
                     return item->cached.ownerEditorID;
                  case Column::Health:
                     return item->health;
               }
               break;
            case Qt::TextAlignmentRole:
               switch (column) {
                  case Column::Level:
                  case Column::Count:
                     return (int)(Qt::AlignRight | Qt::AlignBaseline);
               }
               break;
         }
         return {};
      }
      /*virtual*/ Qt::ItemFlags LeveledListModel::flags(const QModelIndex& index) const /*override*/ {
         if (!index.isValid())
            return Qt::ItemIsDropEnabled;
         return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemNeverHasChildren;
      }
   #pragma endregion
   #pragma region Editing
      /*virtual*/ bool LeveledListModel::insertRows(int row, int count, const QModelIndex& parent) /*override*/ {
         if (parent.isValid())
            return false; // items are not allowed to have children
         if (row < 0 || count < 1)
            return false;
         auto& list = this->_items;
         if (list.size() + count > backend_type::max_entry_count)
            return false;

         this->beginInsertRows({}, row, row + count - 1);
         list.insert(list.begin() + row, count, nullptr);
         assert(list[row]             == nullptr);
         assert(list[row + count - 1] == nullptr);
         for (size_t i = 0; i < count; ++i) {
            list[row + i] = new LeveledObject;
         }
         this->endInsertRows();
         return true;
      }
      /*virtual*/ bool LeveledListModel::removeRows(int row, int count, const QModelIndex& parent) /*override*/ {
         if (parent.isValid())
            return false; // items are not allowed to have children, so there are no children to remove
         if (row < 0 || count < 1 || row + count > this->_items.size())
            return false;
         this->beginRemoveRows({}, row, row + count - 1);
         for (size_t i = 0; i < count; ++i) {
            delete this->_items[row + i];
         }
         this->_items.erase(this->_items.begin() + row, this->_items.begin() + row + count);
         this->endRemoveRows();
         return true;
      }

      /*virtual*/ bool LeveledListModel::setData(const QModelIndex& index, const QVariant& value, int role) /*override*/ {
         if (!index.isValid())
            return {};
         auto item    = (LeveledObject*)index.internalPointer();
         auto column  = index.column();
         if (column >= this->columnCount())
            return {};

         bool changed = false;

         std::optional<uint16_t>          value_uint16_t;
         std::optional<float>             value_float;
         std::optional<dovah::form_stub*> value_form = nullptr;
         if (value.canConvert<int>()) {
            auto i = value.value<int>();
            if (i >= 0 && i <= std::numeric_limits<uint16_t>::max())
               value_uint16_t = i;
         }
         if (value.canConvert<float>()) {
            value_float = value.value<float>();
         }
         if (value.canConvert<dovah::form_stub*>()) {
            value_form = value.value<dovah::form_stub*>();
         }

         switch (column) {
            case Column::Level:
               if (value_uint16_t.has_value()) {
                  item->level = value_uint16_t.value();
                  changed = true;
               }
               break;
            case Column::Count:
               if (value_uint16_t.has_value()) {
                  item->count = value_uint16_t.value();
                  changed = true;
               }
               break;
            case Column::Form:
               if (value_form.has_value()) {
                  auto* stub = value_form.value();
                  if (stub) {
                     item->form = stub;
                     item->cached.editorID = QString::fromStdString(stub->editorID);
                     changed = true;
                  }
               }
               break;
            case Column::Owner:
               if (value_form.has_value()) {
                  auto* stub = value_form.value();
                  item->ownership.owner = stub;
                  if (stub) {
                     item->cached.ownerEditorID = QString::fromStdString(stub->editorID);
                  } else {
                     item->cached.ownerEditorID.clear();
                  }
                  changed = true;
               }
               break;
            case Column::Health:
               if (value_float.has_value()) {
                  item->health = value_float.value();
                  changed = true;
               }
               break;
         }
         if (changed) {
            emit dataChanged(index, index, typical_roles_to_notify_changes_for);
            return true;
         }
         return false;
      }
   #pragma endregion
#pragma endregion

void LeveledListModel::importFrom(const backend_type& component) {
   this->beginResetModel();

   for(auto* item : this->_items)
      delete item;
   this->_items.clear();

   this->_allowed_form_types = component.legal_form_types();

   this->_items.reserve(component.entries.size());
   for (const auto& src : component.entries) {

      // Strip out illegal entries.
      if (!src.form)
         continue;
      if (!component.allows_form_type(src.form.get_form_stub()->form_type))
         continue;

      auto* dst = new LeveledObject;
      this->_items.push_back(dst);
      dst->count = src.count;
      dst->level = src.level;
      dst->form  = src.form.get_form_stub();
      if (auto& src_coed_opt = src.item_extra_data; src_coed_opt.has_value()) {
         auto& src_coed = src_coed_opt.value();

         dst->health = src_coed.health;

         dst->ownership.owner  = src_coed.ownership.get_owner();
         dst->ownership.global = src_coed.ownership.get_global();
         dst->ownership.rank   = src_coed.ownership.get_rank();
      }
   }

   this->endResetModel();
}
void LeveledListModel::commitTo(loaded_form& component_containing_form, backend_type& component) const {
   auto& src_list = this->_items;
   auto& dst_list = component.entries;

   size_t size = src_list.size();
   if (dst_list.size() < size)
      dst_list.resize(size);

   for (size_t i = 0; i < size; ++i) {
      auto& src = *src_list[i];
      auto& dst = dst_list[i];
      dst.count = src.count;
      dst.level = src.level;
      dst.form.set(component_containing_form, src.form);
      {
         auto& dst_coed_opt = dst.item_extra_data;
         if (src.ownership.owner || src.health != normal_item_health) {
            if (!dst_coed_opt.has_value()) {
               dst_coed_opt.emplace();
            }
            auto& dst_coed = dst_coed_opt.value();

            dst_coed.health = src.health;
            dst_coed.ownership.set_owner(component_containing_form, src.ownership.owner);
            dst_coed.ownership.set_global(component_containing_form, src.ownership.global);
            dst_coed.ownership.set_rank(component_containing_form, src.ownership.rank);
         } else {
            if (dst_coed_opt.has_value()) {
               dst_coed_opt.value().ownership.set_owner(component_containing_form, nullptr);
               dst_coed_opt = {};
            }
         }
      }
   }
   if (size < dst_list.size()) {
      // Safe list shrink (i.e. make sure we maintain use info properly)
      for (size_t i = size; i < dst_list.size(); ++i) {
         dst_list[i].form.set(component_containing_form, nullptr);
      }
      dst_list.resize(size);
   }
}

void LeveledListModel::setShowsContainerItemFields(bool v) {
   if (this->showsContainerItemFields() == v)
      return;
   if (v) {
      this->beginInsertColumns({}, Column::_COUNT_ALWAYS, Column::_COUNT_IF_ITEMS - 1);
      this->endInsertColumns();
   } else {
      this->beginRemoveColumns({}, Column::_COUNT_ALWAYS, Column::_COUNT_IF_ITEMS - 1);
      this->endRemoveColumns();
   }
}

void LeveledListModel::setData(size_t row, const LeveledObject& src) {
   if (row >= this->_items.size())
      return;
   auto& dst = *this->_items[row];

   size_t col_change_start = std::numeric_limits<size_t>::max();
   size_t col_change_end   = 0;
   auto _update_column_range = [&col_change_start, &col_change_end](int col) {
      if (col_change_start > col)
         col_change_start = col;
      if (col_change_end < col)
         col_change_end = col;
   };

   if (src.count != dst.count) {
      dst.count = src.count;
      _update_column_range(Column::Count);
   }
   if (src.level != dst.level) {
      dst.level = src.level;
      _update_column_range(Column::Level);
   }
   if (src.form != dst.form) {
      if (dst.form && this->allowsFormType(dst.form->form_type)) {
         dst.form = src.form;
         _update_column_range(Column::Form);
      }
   }
   if (src.ownership.owner != dst.ownership.owner) {
      dst.ownership.owner = src.ownership.owner;
      _update_column_range(Column::Owner);
   }
   dst.ownership.global = src.ownership.global;
   dst.ownership.rank   = src.ownership.rank;
   if (src.health != dst.health) {
      dst.health = src.health;
      _update_column_range(Column::Health);
   }

   if (col_change_start > col_change_end)
      //
      // Nothing actually changed.
      //
      return;

   auto tl = this->index(row, col_change_start, {});
   auto br = this->index(row, col_change_end,   {});
   emit dataChanged(tl, br, typical_roles_to_notify_changes_for);
}

void LeveledListModel::_clear() {
   this->beginResetModel();
   for (auto* item : this->_items)
      delete item;
   this->_items.clear();
   this->endResetModel();
}