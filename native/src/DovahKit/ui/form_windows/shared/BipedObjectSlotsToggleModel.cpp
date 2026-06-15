#include "./BipedObjectSlotsToggleModel.h"
#include <QFont>
#include "dovah/forms/components/biped_object.h"
#include "dovah/forms/Race.h"
#include "editor/core.h"

BipedObjectSlotsToggleModel::BipedObjectSlotsToggleModel(QObject* parent) : QAbstractItemModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() {
      this->_names_set_from = nullptr;
   });
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub) {
      if (this->_names_set_from == stub)
         this->_names_set_from = nullptr;
   });
   QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) {
      if (this->_names_set_from && this->_names_set_from == stub)
         this->setSlotNamesFrom(*this->_names_set_from);
   });
}
   
#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex BipedObjectSlotsToggleModel::index(int row, int col, const QModelIndex& parent) const /*override*/ {
         if (row < 0 || row >= this->_slots.size())
            return {};
         if (col < 0 || col >= this->columnCount())
            return {};
         if (parent.isValid())
            return {};
         return this->createIndex(row, col, nullptr);
      }
      /*virtual*/ QModelIndex BipedObjectSlotsToggleModel::parent(const QModelIndex& index) const /*override*/ {
         return {};
      }
      /*virtual*/ QModelIndex BipedObjectSlotsToggleModel::sibling(int row, int column, const QModelIndex& index) const /*override*/ {
         if (!index.isValid())
            return {};
         return this->index(row, column, {});
      }
      /*virtual*/ int BipedObjectSlotsToggleModel::rowCount(const QModelIndex& parent) const /*override*/ {
         return this->_slots.size();
      }
      /*virtual*/ int BipedObjectSlotsToggleModel::columnCount(const QModelIndex& parent) const /*override*/ {
         return ColumnCount;
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant BipedObjectSlotsToggleModel::data(const QModelIndex& index, int role) const /*override*/ {
         if (!index.isValid())
            return {};
         if (index.row() >= this->_slots.size())
            return {};
         auto& src = this->_slots[index.row()];
         switch (role) {
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
               if (src.name.isEmpty()) {
                  return tr("<unnamed>");
               }
               return src.name;
            case Qt::FontRole:
               if (src.name.isEmpty()) {
                  QFont font;
                  font.setItalic(true);
                  return font;
               }
               return {};
            case Qt::CheckStateRole:
               return src.enabled ? Qt::CheckState::Checked : Qt::CheckState::Unchecked;
         }
         return {};
      }
      /*virtual*/ Qt::ItemFlags BipedObjectSlotsToggleModel::flags(const QModelIndex& index) const /*override*/ {
         auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
         flags |= Qt::ItemFlag::ItemIsUserCheckable;
         return flags;
      }
      #pragma region Write-access
         /*virtual*/ bool BipedObjectSlotsToggleModel::setData(const QModelIndex& index, const QVariant& value, int role) /*override*/ {
            if (!index.isValid() || index.row() >= slot_count || index.column() >= ColumnCount)
               return false;
            if (role != Qt::CheckStateRole)
               return false;
            if (value.userType() != QMetaType::Bool)
               return false;
            auto& dst = this->_slots[index.row()];
            dst.enabled = value.toBool();
            return true;
         }
      #pragma endregion
   #pragma endregion
      /*virtual*/ QVariant BipedObjectSlotsToggleModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
         if (orientation != Qt::Orientation::Horizontal)
            return {};
         if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
            return {};
         if (section != 0)
            return {};
         return tr("Biped Object Slot");
      }
#pragma endregion
      
void BipedObjectSlotsToggleModel::setSlotNamesFrom(dovah::form_stub& stub) {
   switch (stub.form_type) {
      case dovah::form_type::race:
         {
            auto loaded = stub.load().ptr_cast<dovah::loaded_forms::Race>();
            if (loaded) {
               auto& names = loaded->biped_object_info.names;
               for (size_t i = 0; i < names.size(); ++i) {
                  this->_slots[i].name = QString::fromStdString(names[i]);
               }
               this->_names_set_from = &stub;

               auto dummy = QModelIndex{};
               auto tl    = this->index(0, 0, dummy);
               auto br    = this->index(slot_count - 1, ColumnCount - 1, dummy);
               emit dataChanged(tl, br, { Qt::DisplayRole, Qt::ToolTipRole });
            }
         }
         break;
   }
}

void BipedObjectSlotsToggleModel::importFlags(const dovah::loaded_forms::components::biped_object& bod2) {
   for (size_t i = 0; i < slot_count; ++i) {
      auto& dst = this->_slots[i];
      dst.enabled = bod2.first_person_slots & (1 << i);
   }
   auto dummy = QModelIndex{};
   auto tl    = this->index(0, 0, dummy);
   auto br    = this->index(slot_count - 1, ColumnCount - 1, dummy);
   emit dataChanged(tl, br);
}
void BipedObjectSlotsToggleModel::exportFlags(dovah::loaded_forms::components::biped_object& bod2) const {
   for (size_t i = 0; i < slot_count; ++i) {
      auto& src = this->_slots[i];
      {
         auto mask = 1 << i;
         if (src.enabled) {
            bod2.first_person_slots |= mask;
         } else {
            bod2.first_person_slots &= ~mask;
         }
      }
   }
}