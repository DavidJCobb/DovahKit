#include "./DKMagicEffectListModel.h"
#include <limits>
#include "dovah/data/actor_values.h"
#include "dovah/forms/components/magic_effect_list.h"
#include "dovah/form_stub.h"
#include "editor/core.h"

/*// TODO: When we can load AVIFs, use their TESFullNames.
#include "dovah/forms/ActorValue.h"
//*/
#include "dovah/forms/MagicEffect.h"

void DKMagicEffectListModel::Item::_update_cache() {
   if (!this->magic_effect) {
      this->cached = {};
      return;
   }
   auto loaded = this->magic_effect->load().ptr_cast<dovah::loaded_forms::MagicEffect>();
   if (!loaded) {
      this->cached = {};
      return;
   }
   
   auto& editor = DovahKitCore::get();
   this->cached.base_cost   = loaded->base_cost;
   this->cached.effect_name = editor.convert_localized_string(loaded->name);
   {  // Magic School name
      auto i = loaded->magic_skill;
      if (i < 0) {
         this->cached.magic_school = tr("NONE", "actor vale name");
      } else if (i >= dovah::all_actor_value_info.size()) {
         this->cached.magic_school = tr("INVALID", "actor vale name");
      } else {
         auto&   av_info = dovah::all_actor_value_info[i];
         auto*   av_stub = editor.get_form_of_probable_type(dovah::form_type::actor_value_info, av_info.formID);
         QString av_name;
         if (av_stub) {
            /*// TODO: When we can load AVIFs, use their TESFullNames.
            auto loaded = av_stub->load().ptr_cast<dovah::loaded_forms::ActorValue>();
            if (loaded) {
               av_name = editor.convert_localized_string(loaded->name);
            }
            //*/
         }
         if (av_name.isEmpty()) {
            this->cached.magic_school = QString(QLatin1String(av_info.name.data(), av_info.name.size()));
         } else {
            this->cached.magic_school = av_name;
         }
      }
   }
}

DKMagicEffectListModel::DKMagicEffectListModel(QObject* parent) : QAbstractItemModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() {
      this->_clear();
   });
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub, bool just_being_flagged) {
      auto&  list = this->_items;
      size_t size = list.size();
      for (size_t i = 0; i < size; ++i) {
         auto& item = *list[i];
         if (item.magic_effect == stub) {
            this->beginRemoveRows({}, i, i);
            delete list[i];
            list.erase(list.begin() + i);
            --i;
            --size;
            this->endRemoveRows();
            continue;
         }
      }
   });
   QObject::connect(&editor, &DovahKitCore::formModified, this, [this](const dovah::form_stub* stub) {
      for (size_t i = 0; i < this->_items.size(); ++i) {
         auto& item = *this->_items[i];
         if (stub == item.magic_effect) {
            item._update_cache();

            QModelIndex tl = this->index(i, Column::Name, {});
            QModelIndex br = this->index(i, Column::Name, {});
            emit dataChanged(tl, br, { Qt::DisplayRole, Qt::ToolTipRole });
            tl = this->index(i, Column::Cost, {});
            br = this->index(i, Column::Cost, {});
            emit dataChanged(tl, br, { Qt::DisplayRole, Qt::ToolTipRole });
         }
      }
   });
}
DKMagicEffectListModel::~DKMagicEffectListModel() {
   this->_clear();
}
      
#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex DKMagicEffectListModel::index(int row, int column, const QModelIndex& parent) const /*override*/ {
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
      /*virtual*/ QModelIndex DKMagicEffectListModel::parent(const QModelIndex& index) const /*override*/ {
         return {};
      }
      /*virtual*/ QModelIndex DKMagicEffectListModel::sibling(int row, int column, const QModelIndex& index) const /*override*/ {
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
      /*virtual*/ int DKMagicEffectListModel::rowCount(const QModelIndex& parent) const /*override final*/ {
         return this->_items.size();
      }
      /*virtual*/ int DKMagicEffectListModel::columnCount(const QModelIndex& item) const /*override final*/ {
         return Column::_COUNT;
      }

      /*virtual*/ QVariant DKMagicEffectListModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
         if (orientation != Qt::Orientation::Horizontal)
            return {};
         if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
            return {};
         switch (section) {
            case Column::Index:
               return tr("Index", "column header");
            case Column::Name:
               return tr("Effect Name", "column header");
            case Column::Magnitude:
               return tr("Magnitude", "column header");
            case Column::Area:
               return tr("Area", "column header");
            case Column::Duration:
               return tr("Duration", "column header");
            case Column::Cost:
               return tr("Cost", "column header");
            case Column::MagicSchool:
               return tr("Magic School", "column header");
         }
         return {};
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant DKMagicEffectListModel::data(const QModelIndex& index, int role) const /*override*/ {
         if (!index.isValid())
            return {};
         auto item   = (Item*)index.internalPointer();
         auto column = index.column();
         switch (role) {
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
               switch (column) {
                  case Column::Index:
                     return index.row();
                  case Column::Name:
                     return item->cached.effect_name;
                  case Column::Magnitude:
                     return item->magnitude;
                  case Column::Area:
                     return item->area;
                  case Column::Duration:
                     return item->duration;
                  case Column::Cost:
                     return item->cached.base_cost;
                  case Column::MagicSchool:
                     return item->cached.magic_school;
               }
               break;
            case Qt::TextAlignmentRole:
               switch (column) {
                  case Column::Index:
                  case Column::Magnitude:
                  case Column::Area:
                  case Column::Duration:
                  case Column::Cost:
                     return (int)(Qt::AlignRight | Qt::AlignVCenter);
               }
               break;
         }
         return {};
      }
      /*virtual*/ Qt::ItemFlags DKMagicEffectListModel::flags(const QModelIndex& index) const /*override*/ {
         if (!index.isValid())
            return {};
         return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemNeverHasChildren;
      }
   #pragma endregion
   #pragma region Editing
      /*virtual*/ bool DKMagicEffectListModel::insertRows(int row, int count, const QModelIndex& parent) /*override*/ {
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
            auto* item = list[row + i] = new Item;
         }
         this->endInsertRows();
         return true;
      }
      /*virtual*/ bool DKMagicEffectListModel::removeRows(int row, int count, const QModelIndex& parent) /*override*/ {
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
   #pragma endregion
#pragma endregion

void DKMagicEffectListModel::importFrom(loaded_form& component_containing_form, const backend_type& component) {
   this->beginResetModel();

   for (auto* item : this->_items)
      delete item;
   this->_items.clear();

   this->_condition_context = ui::types::conditions::context(component_containing_form.stub, component_containing_form.is_working_copy);
   this->_items.reserve(component.items.size());
   for (const auto& src : component.items) {

      // Strip out illegal entries.
      if (!src.effect)
         continue;
      if (src.effect.get_form_stub()->form_type != dovah::form_type::magic_effect)
         continue;

      auto* dst = new Item;
      this->_items.push_back(dst);
      dst->magic_effect = src.effect.get_form_stub();
      dst->area         = src.area;
      dst->duration     = src.duration;
      dst->magnitude    = src.magnitude;
      for (auto& src_cnd : src.conditions) {
         dst->conditions.emplace_back(src_cnd);
      }
      dst->_update_cache();
   }

   this->endResetModel();
}
void DKMagicEffectListModel::commitTo(loaded_form& component_containing_form, backend_type& component) const {
   auto& src_list = this->_items;
   auto& dst_list = component.items;

   size_t size = src_list.size();
   if (dst_list.size() < size) {
      dst_list.resize(size);
   }

   for (size_t i = 0; i < size; ++i) {
      auto& src = *src_list[i];
      auto& dst = dst_list[i];
      dst.effect.set(component_containing_form, src.magic_effect);
      dst.area      = src.area;
      dst.duration  = src.duration;
      dst.magnitude = src.magnitude;
      dst.conditions.clear(component_containing_form);
      dst.conditions.append_all_of(component_containing_form, src.conditions);
   }
   if (size < dst_list.size()) {
      // Safe list shrink (i.e. make sure we maintain use info properly)
      for (size_t i = size; i < dst_list.size(); ++i) {
         dst_list[i].effect.set(component_containing_form, nullptr);
         dst_list[i].conditions.clear(component_containing_form);
      }
      dst_list.resize(size);
   }
}

void DKMagicEffectListModel::setData(size_t row, const Item& src) {
   if (row >= this->_items.size())
      return;
   auto& dst = *this->_items[row];

   dst = src;
   dst._update_cache();

   auto tl = this->index(row, 0, {});
   auto br = this->index(row, Column::_COUNT - 1, {});
   emit dataChanged(tl, br, { Qt::DisplayRole, Qt::ToolTipRole });
}

void DKMagicEffectListModel::_clear() {
   this->beginResetModel();
   for (auto* item : this->_items)
      delete item;
   this->_items.clear();
   this->endResetModel();
}