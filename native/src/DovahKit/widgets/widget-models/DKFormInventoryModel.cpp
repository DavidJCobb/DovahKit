#include "./DKFormInventoryModel.h"
#include <limits>
#include "dovah/data/all_carryable_form_types.h"
#include "dovah/data/all_item_form_types.h"
#include "dovah/forms/components/container.h"
#include "dovah/form_stub.h"
#include "editor/core.h"
#include "editor/helpers/form_stub_drag_drop.h"

namespace {
   static const QVector<int> typical_roles_to_notify_changes_for = { Qt::DisplayRole, Qt::ToolTipRole };

   // Set to `true` to insert dropped items adjacent to whatever existing list item 
   // the user dropped them at. Set to `false` to always append dropped items.
   constexpr const bool do_we_care_about_drop_position = false;
}

#include "dovah/forms/Light.h"
#include "dovah/forms/MiscItem.h"
namespace {
   int32_t _get_form_value(dovah::form_stub& stub) {
      switch (stub.form_type) {
         case dovah::form_type::leveled_item:
         case dovah::form_type::note:
            return 0;
      }
      auto loaded = stub.load();
      switch (stub.form_type) {
         case dovah::form_type::light:
            if (auto casted = loaded.ptr_cast<dovah::loaded_forms::Light>())
               return casted->item_data.value;
            break;
         case dovah::form_type::misc_item:
            if (auto casted = loaded.ptr_cast<dovah::loaded_forms::MiscItem>())
               return casted->value;
            break;

      }
      return 0;
   }
}

DKFormInventoryModel::DKFormInventoryModel(QObject* parent) : QAbstractItemModel(parent) {
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
            item.cached.value = 0;
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
               item.cached.value    = _get_form_value(*(dovah::form_stub*)stub);
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
DKFormInventoryModel::~DKFormInventoryModel() {
   this->_clear();
}
      
#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex DKFormInventoryModel::index(int row, int column, const QModelIndex& parent) const /*override*/ {
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
      /*virtual*/ QModelIndex DKFormInventoryModel::parent(const QModelIndex& index) const /*override*/ {
         return {};
      }
      /*virtual*/ QModelIndex DKFormInventoryModel::sibling(int row, int column, const QModelIndex& index) const /*override*/ {
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
      /*virtual*/ int DKFormInventoryModel::rowCount(const QModelIndex& parent) const /*override final*/ {
         return this->_items.size();
      }
      /*virtual*/ int DKFormInventoryModel::columnCount(const QModelIndex& item) const /*override final*/ {
         if (!this->_allow_extra_data)
            return Column::_COUNT - Column::_COUNT_EXTRA;
         return Column::_COUNT;
      }

      /*virtual*/ QVariant DKFormInventoryModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
         if (orientation != Qt::Orientation::Horizontal)
            return {};
         if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
            return {};
         if (!this->_allow_extra_data) {
            if (section >= Column::_FIRST_EXTRA)
               section += Column::_COUNT_EXTRA;
         }
         switch (section) {
            case Column::Count:
               return tr("Count", "column header");
            case Column::Form:
               return tr("Form Editor ID", "column header");
            case Column::Owner:
               return tr("Owner", "column header");
            case Column::Health:
               return tr("Health", "column header");
            case Column::Value:
               return tr("Value", "column header");
         }
         return {};
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant DKFormInventoryModel::data(const QModelIndex& index, int role) const /*override*/ {
         if (!index.isValid())
            return {};
         auto item   = (InventoryObject*)index.internalPointer();
         auto column = index.column();
         if (!this->_allow_extra_data) {
            if (column >= Column::_FIRST_EXTRA)
               column += Column::_COUNT_EXTRA;
         }
         switch (role) {
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
               switch (column) {
                  case Column::Count:
                     return item->count;
                  case Column::Form:
                     return item->cached.editorID;
                  case Column::Owner:
                     return item->cached.ownerEditorID;
                  case Column::Health:
                     return item->health * health_display_mult;
                  case Column::Value:
                     if (auto* stub = item->form) {
                        if (stub->form_type == dovah::form_type::leveled_item) {
                           return tr("?", "value column for LeveledItem");
                        }
                     }
                     return item->cached.value * item->count;
               }
               break;
            case Qt::TextAlignmentRole:
               switch (column) {
                  case Column::Count:
                  case Column::Value:
                     return (int)(Qt::AlignRight | Qt::AlignVCenter);
               }
               break;
         }
         return {};
      }
      /*virtual*/ Qt::ItemFlags DKFormInventoryModel::flags(const QModelIndex& index) const /*override*/ {
         if (!index.isValid())
            return Qt::ItemIsDropEnabled;
         return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemNeverHasChildren;
      }
   #pragma endregion
   #pragma region Editing
      /*virtual*/ bool DKFormInventoryModel::insertRows(int row, int count, const QModelIndex& parent) /*override*/ {
         if (parent.isValid())
            return false; // items are not allowed to have children
         if (row < 0 || count < 1)
            return false;
         auto& list = this->_items;

         this->beginInsertRows({}, row, row + count - 1);
         list.insert(list.begin() + row, count, nullptr);
         assert(list[row]             == nullptr);
         assert(list[row + count - 1] == nullptr);
         for (size_t i = 0; i < count; ++i) {
            auto* item = list[row + i] = new InventoryObject;
         }
         this->endInsertRows();
         return true;
      }
      /*virtual*/ bool DKFormInventoryModel::removeRows(int row, int count, const QModelIndex& parent) /*override*/ {
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
      #pragma region Drag-and-drop
         bool DKFormInventoryModel::canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const {
            if (!data->hasFormat(editor_helpers::form_stub_array_mime_type))
               return false;
            return true;
         }
         bool DKFormInventoryModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) {
            if (!this->canDropMimeData(data, action, row, column, parent))
               return false;
            if (action == Qt::IgnoreAction)
               return true;
            if (row == -1) {
               row = this->_items.size();
               if constexpr (do_we_care_about_drop_position) {
                  if (parent.isValid())
                     row = parent.row();
               }
            }

            auto dropped_stubs = editor_helpers::form_stubs_from_mime_data(*data);
            std::erase_if(dropped_stubs, [this](dovah::form_stub* stub) {
               if (!stub)
                  return true;
               if (!this->_item_type_is_allowed(stub->form_type))
                  return true;
               return false;
            });
            const size_t size = dropped_stubs.size();
            if (size) {
               this->beginInsertRows({}, row, row + size - 1);
               this->_items.insert(this->_items.begin() + row, size, nullptr);
               for(size_t i = 0; i < size; ++i) {
                  auto* stub = dropped_stubs[i];
                  auto* item = this->_items[row + i] = new InventoryObject;
                  item->form = stub;
                  item->cached.value    = _get_form_value(*stub);
                  item->cached.editorID = QString::fromStdString(stub->editorID);
               }
               this->endInsertRows();
            }
            return true;
         }
         QStringList DKFormInventoryModel::mimeTypes() const {
            return QStringList(QString(editor_helpers::form_stub_array_mime_type));
         }
         Qt::DropActions DKFormInventoryModel::supportedDropActions() const {
            return Qt::CopyAction;
         }
      #pragma endregion
   #pragma endregion
#pragma endregion

void DKFormInventoryModel::importFrom(const backend_type& component) {
   this->beginResetModel();

   for(auto* item : this->_items)
      delete item;
   this->_items.clear();

   this->_items.reserve(component.entries.size());
   for (const auto& src : component.entries) {

      // Strip out illegal entries.
      if (!src.item)
         continue;
      if (!this->_item_type_is_allowed(src.item.get_form_stub()->form_type))
         continue;

      auto* dst = new InventoryObject;
      this->_items.push_back(dst);
      dst->count = src.count;
      dst->form  = src.item.get_form_stub();
      if (auto& src_coed = src.extra_data; src_coed.is_default()) {
         dst->health = src_coed.health;

         dst->ownership.owner  = src_coed.ownership.get_owner();
         dst->ownership.global = src_coed.ownership.get_global();
         dst->ownership.rank   = src_coed.ownership.get_rank();
      }

      if (auto* stub = dst->form) {
         dst->cached.value    = _get_form_value(*stub);
         dst->cached.editorID = QString::fromStdString(stub->editorID);
      }
      if (auto* stub = dst->ownership.owner)
         dst->cached.ownerEditorID = QString::fromStdString(stub->editorID);
   }

   this->endResetModel();
}
void DKFormInventoryModel::commitTo(loaded_form& component_containing_form, backend_type& component) const {
   auto& src_list = this->_items;
   auto& dst_list = component.entries;

   size_t size = src_list.size();
   if (dst_list.size() < size)
      dst_list.resize(size);

   for (size_t i = 0; i < size; ++i) {
      auto& src = *src_list[i];
      auto& dst = dst_list[i];
      dst.count = src.count;
      dst.item.set(component_containing_form, src.form);
      {
         auto& dst_coed = dst.extra_data;
         dst_coed.health = src.health;
         dst_coed.ownership.set_owner(component_containing_form, src.ownership.owner);
         dst_coed.ownership.set_global(component_containing_form, src.ownership.global);
         dst_coed.ownership.set_rank(component_containing_form, src.ownership.rank);
      }
   }
   if (size < dst_list.size()) {
      // Safe list shrink (i.e. make sure we maintain use info properly)
      for (size_t i = size; i < dst_list.size(); ++i) {
         dst_list[i].item.set(component_containing_form, nullptr);
      }
      dst_list.resize(size);
   }
}

void DKFormInventoryModel::setData(size_t row, const InventoryObject& src) {
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
   if (src.form != dst.form) {
      if (src.form && this->_item_type_is_allowed(src.form->form_type)) {
         dst.form = src.form;
         dst.cached.value    = _get_form_value(*src.form);
         dst.cached.editorID = QString::fromStdString(dst.form->editorID);
         _update_column_range(Column::Form);
      }
   }
   if (src.ownership.owner != dst.ownership.owner) {
      dst.ownership.owner = src.ownership.owner;
      if (auto* stub = dst.ownership.owner)
         dst.cached.ownerEditorID = QString::fromStdString(stub->editorID);
      _update_column_range(Column::Owner);
   }
   dst.ownership.global = src.ownership.global;
   dst.ownership.rank   = src.ownership.rank;
   if (src.health != dst.health) {
      dst.health = src.health;
      _update_column_range(Column::Health);
   }

   if (col_change_start > col_change_end) {
      //
      // Nothing actually changed.
      //
      return;
   }
   if (!this->_allow_extra_data) {
      if (col_change_start >= Column::_FIRST_EXTRA)
         col_change_start -= Column::_COUNT_EXTRA;
      if (col_change_end >= Column::_FIRST_EXTRA)
         col_change_end -= Column::_COUNT_EXTRA;
   }

   auto tl = this->index(row, col_change_start, {});
   auto br = this->index(row, col_change_end,   {});
   emit dataChanged(tl, br, typical_roles_to_notify_changes_for);
}

bool DKFormInventoryModel::allowsExtraData() const {
   return this->_allow_extra_data;
}
void DKFormInventoryModel::setAllowsExtraData(bool v) {
   if (v == this->_allow_extra_data)
      return;
   if (v) {
      this->beginInsertColumns({}, Column::_FIRST_EXTRA, Column::_LAST_EXTRA);
   } else {
      this->beginRemoveColumns({}, Column::_FIRST_EXTRA, Column::_LAST_EXTRA);
   }
   this->_allow_extra_data = v;
   if (v) {
      this->endInsertColumns();
   } else {
      this->endRemoveColumns();
   }
}

bool DKFormInventoryModel::allowsPseudoItems() const {
   return this->_allow_pseudo_items;
}
void DKFormInventoryModel::setAllowsPseudoItems(bool v) {
   if (v == this->_allow_pseudo_items)
      return;
   this->_allow_pseudo_items = v;
   if (!v) {
      constexpr auto pseudo_item_types = []() {
         constexpr size_t count = dovah::all_carryable_form_types.size() - dovah::all_item_form_types.size();
         std::array<dovah::form_type, count> types = {};

         size_t i = 0;
         for (auto a : dovah::all_carryable_form_types) {
            bool found = false;
            for (auto b : dovah::all_item_form_types) {
               if (a == b) {
                  found = true;
                  break;
               }
            }
            if (!found)
               types[i++] = a;
         }

         return types;
      }();

      size_t size = this->_items.size();
      for (size_t i = 0; i < size; ++i) {
         auto* item = this->_items[i];
         if (!item->form)
            continue;
         bool remove = false;
         for (auto ft : pseudo_item_types) {
            if (item->form->form_type == ft) {
               remove = true;
               break;
            }
         }
         if (remove) {
            this->beginRemoveRows({}, i, i);
            this->_items.erase(this->_items.begin() + i);
            this->endRemoveRows();
            --i;
            --size;
         }
      }
   }
}

void DKFormInventoryModel::_clear() {
   this->beginResetModel();
   for (auto* item : this->_items)
      delete item;
   this->_items.clear();
   this->_allow_extra_data = true;
   this->endResetModel();
}
bool DKFormInventoryModel::_item_type_is_allowed(dovah::form_type ft) const {
   if (this->_allow_pseudo_items) {
      for (auto allowed : dovah::all_carryable_form_types)
         if (ft == allowed)
            return true;
   } else {
      for (auto allowed : dovah::all_item_form_types)
         if (ft == allowed)
            return true;
   }
   return false;
}