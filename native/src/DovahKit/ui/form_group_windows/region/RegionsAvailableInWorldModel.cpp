#include "./RegionsAvailableInWorldModel.h"
#include "helpers/bound_mem_fn.h"
#include "dovah/form_stub.h"
#include "dovah/form_stubs/helpers/for_each_inbound_use_with_flag.h"
#include "dovah/form_stubs/helpers/get_unique_outbound_use.h"
#include "dovah/use_info/entry_flags/region.h"
#include "dovah/utils/get_region_worldspace.h"
#include "editor/core.h"
#include "editor/form_stub_meta_type.h"

RegionsAvailableInWorldModel::RegionsAvailableInWorldModel(QObject* parent) : QAbstractItemModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent,  this, cobb__bound_this_fn(_on_data_abandoned));
   QObject::connect(&editor, &DovahKitCore::dataAcquireComplete,  this, cobb__bound_this_fn(_on_data_acquired));
   QObject::connect(&editor, &DovahKitCore::formCreated,          this, [this](dovah::form_stub* stub) { this->_on_form_created(*stub); });
   QObject::connect(&editor, &DovahKitCore::formModified,         this, [this](dovah::form_stub* stub) { this->_on_form_modified(*stub); });
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub) { this->_on_form_deleted(*stub); });
   if (editor.has_data()) {
      this->_gather_regions();
   }
}

/*static*/ bool RegionsAvailableInWorldModel::_sort_comparator(const Item& a, const Item& b) {
   return a.editor_id.localeAwareCompare(b.editor_id) < 0;
}

void RegionsAvailableInWorldModel::_gather_regions() {
   this->beginResetModel();

   this->_data.clear();
   DovahKitCore::get().for_each_form_of_type(dovah::form_type::region, [this](dovah::form_stub* region) -> bool {
      auto* world = dovah::utils::get_region_worldspace(*region);
      if (!world || world == this->_worldspace) {
         auto& item = this->_data.emplace_back(region);
         this->_recache(item);
      }
      return false;
   });
   std::sort(this->_data.begin(), this->_data.end(), &_sort_comparator);

   this->endResetModel();
}
void RegionsAvailableInWorldModel::_insert_region(dovah::form_stub& stub) {
   Item item{ &stub };
   this->_recache(item);
   _do_sorted_insertion(std::move(item));
}
void RegionsAvailableInWorldModel::_recache(Item& item) {
   if (item.stub) {
      item.editor_id = QString::fromStdString(item.stub->editorID);
   } else {
      item.editor_id = tr("NONE");
   }
}

#pragma region Editor events
   void RegionsAvailableInWorldModel::_on_form_created(dovah::form_stub& stub) {
      if (stub.form_type != dovah::form_type::region)
         return;
      _insert_region(stub);
   }
   void RegionsAvailableInWorldModel::_on_form_modified(dovah::form_stub& stub) {
      if (stub.form_type == dovah::form_type::region) {
         auto* world         = dovah::form_stub_helpers::get_unique_outbound_use<dovah::use_info::entry_flags::region::worldspace>(stub);
         bool  in_same_world = !this->_worldspace || !world || this->_worldspace == world;
         for (size_t i = 0; i < this->_data.size(); ++i) {
            auto& item = this->_data[i];
            if (item.stub == &stub) {
               if (!in_same_world) {
                  this->beginRemoveRows({}, i, i);
                  this->_data.erase(this->_data.begin() + i);
                  this->endRemoveRows();
                  return;
               }
               this->_recache(item);
               auto qmi = this->index(i, 0, {});
               emit dataChanged(qmi, qmi);
               this->_re_sort_item(i);
               return;
            }
         }
         //
         // Else the region isn't in our model (yet?).
         //
         if (in_same_world) {
            _insert_region(stub);
         }
         return;
      }
   }
   void RegionsAvailableInWorldModel::_on_form_deleted(const dovah::form_stub& stub) {
      if (stub.form_type == dovah::form_type::worldspace) {
         if (&stub == this->_worldspace) {
            this->setWorldspace(nullptr);
         }
         return;
      }
      if (stub.form_type == dovah::form_type::region) {
         for (size_t i = 0; i < this->_data.size(); ++i) {
            auto& item = this->_data[i];
            if (item.stub == &stub) {
               this->beginRemoveRows({}, i, i);
               this->_data.erase(this->_data.begin() + i);
               this->endRemoveRows();
               break;
            }
         }
         return;
      }
   }
   void RegionsAvailableInWorldModel::_on_data_acquired() {
      this->_gather_regions();
   }
   void RegionsAvailableInWorldModel::_on_data_abandoned() {
      this->beginResetModel();
      this->_data.clear();
      this->_worldspace = nullptr;
      this->endResetModel();
   }
#pragma endregion

void RegionsAvailableInWorldModel::setWorldspace(dovah::form_stub* stub) {
   if (stub && stub->form_type != dovah::form_type::worldspace)
      return;
   if (stub == this->_worldspace)
      return;
   this->_worldspace = stub;
   this->_gather_regions();
}
QModelIndex RegionsAvailableInWorldModel::regionIndex(const dovah::form_stub& region) const {
   if (region.form_type != dovah::form_type::region)
      return {};
   for (size_t i = 0; i < this->_data.size(); ++i) {
      auto& item = this->_data[i];
      if (item.stub == &region)
         return this->index(i, 0, {});
   }
   return {};
}
   
#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex RegionsAvailableInWorldModel::index(int row, int column, const QModelIndex& parent_qmi) const /*override*/ {
         if (parent_qmi.isValid())
            return {};
         if (row < 0 || column < 0 || column >= ColumnCount)
            return {};
         if (row >= this->_data.size())
            return {};
         return this->createIndex(row, column, nullptr);
      }
      /*virtual*/ QModelIndex RegionsAvailableInWorldModel::parent(const QModelIndex& qmi) const /*override*/ {
         return {};
      }
      /*virtual*/ QModelIndex RegionsAvailableInWorldModel::sibling(int row, int column, const QModelIndex& qmi) const /*override*/ {
         if (!qmi.isValid())
            return {};
         return index(row, column, {});
      }
      /*virtual*/ int RegionsAvailableInWorldModel::rowCount(const QModelIndex& parent_qmi) const /*override*/ {
         if (parent_qmi.isValid())
            return 0;
         return this->_data.size();
      }
      /*virtual*/ int RegionsAvailableInWorldModel::columnCount(const QModelIndex& parent_qmi) const /*override*/ {
         return ColumnCount;
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant RegionsAvailableInWorldModel::data(const QModelIndex& qmi, int role) const /*override*/ {
         if (!qmi.isValid())
            return {};
         if (qmi.row() >= this->_data.size())
            return {};
         auto& item = this->_data[qmi.row()];
         switch (role) {
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
               return item.editor_id;
            case FormStubRole:
               return QVariant::fromValue(item.stub);
         }
         return {};
      }
      /*virtual*/ Qt::ItemFlags RegionsAvailableInWorldModel::flags(const QModelIndex& qmi) const /*override*/ {
         if (!qmi.isValid())
            return {};
         return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
      }
   #pragma endregion
   /*virtual*/ QVariant RegionsAvailableInWorldModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
      if (orientation != Qt::Orientation::Horizontal)
         return {};
      if (section != 0)
         return {};
      switch (role) {
         case Qt::DisplayRole:
         case Qt::ToolTipRole:
            return tr("Region");
      }
      return {};
   }
#pragma endregion