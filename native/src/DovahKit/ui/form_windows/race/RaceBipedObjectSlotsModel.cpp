#include "./RaceBipedObjectSlotsModel.h"
#include "dovah/forms/components/biped_object.h"
#include "dovah/forms/Race.h"

RaceBipedObjectSlotsModel::RaceBipedObjectSlotsModel(QObject* parent) : QAbstractItemModel(parent) {
}
   
#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex RaceBipedObjectSlotsModel::index(int row, int col, const QModelIndex& parent) const /*override*/ {
         if (row < 0 || row >= this->_slots.size())
            return {};
         if (col < 0 || col >= this->columnCount())
            return {};
         if (parent.isValid())
            return {};
         return this->createIndex(row, col, nullptr);
      }
      /*virtual*/ QModelIndex RaceBipedObjectSlotsModel::parent(const QModelIndex& index) const /*override*/ {
         return {};
      }
      /*virtual*/ QModelIndex RaceBipedObjectSlotsModel::sibling(int row, int column, const QModelIndex& index) const /*override*/ {
         if (!index.isValid())
            return {};
         return this->index(row, column, {});
      }
      /*virtual*/ int RaceBipedObjectSlotsModel::rowCount(const QModelIndex& parent) const /*override*/ {
         return this->_slots.size();
      }
      /*virtual*/ int RaceBipedObjectSlotsModel::columnCount(const QModelIndex& parent) const /*override*/ {
         return ColumnCount;
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant RaceBipedObjectSlotsModel::data(const QModelIndex& index, int role) const /*override*/ {
         if (!index.isValid())
            return {};
         if (index.row() >= this->_slots.size())
            return {};
         auto& src = this->_slots[index.row()];
         switch (index.column()) {
            case Column::Name:
               if (role == Qt::DisplayRole || role == Qt::ToolTipRole || role == Qt::UserRole) {
                  return src.name;
               }
               return {};
            case Column::IsFirstPerson:
               if (role == Qt::CheckStateRole) {
                  return src.first_person ? Qt::CheckState::Checked : Qt::CheckState::Unchecked;
               }
               if (role == Qt::UserRole) {
                  return src.first_person;
               }
               return {};
         }
         return {};
      }
      /*virtual*/ Qt::ItemFlags RaceBipedObjectSlotsModel::flags(const QModelIndex& index) const /*override*/ {
         auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
         if (index.column() == Column::IsFirstPerson) {
            flags |= Qt::ItemFlag::ItemIsUserCheckable;
         }
         return flags;
      }
      #pragma region Write-access
         /*virtual*/ bool RaceBipedObjectSlotsModel::setData(const QModelIndex& index, const QVariant& value, int role) /*override*/ {
            if (!index.isValid() || index.row() >= slot_count || index.column() >= ColumnCount)
               return false;
            if (role != Qt::UserRole)
               return false;
            auto& dst = this->_slots[index.row()];
            switch (index.column()) {
               case Column::Name:
                  if (value.userType() != QMetaType::QString)
                     return false;
                  dst.name = value.toString();
                  break;
               case Column::IsFirstPerson:
                  if (value.userType() != QMetaType::Bool)
                     return false;
                  dst.first_person = value.toBool();
                  break;
               default:
                  return false;
            }
            emit dataChanged(index, index, { role });
            return true;
         }
      #pragma endregion
   #pragma endregion
      /*virtual*/ QVariant RaceBipedObjectSlotsModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
         if (orientation != Qt::Orientation::Horizontal)
            return {};
         if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
            return {};
         switch (section) {
            case Column::Name:
               return tr("Name");
            case Column::IsFirstPerson:
               return tr("Visible in First Person");
         }
         return {};
      }
#pragma endregion

void RaceBipedObjectSlotsModel::initializeFrom(const dovah::loaded_forms::Race& race, const dovah::loaded_forms::components::biped_object& bod2) {
   for (size_t i = 0; i < slot_count; ++i) {
      auto& dst = this->_slots[i];
      dst.name         = QString::fromStdString(race.biped_object_info.names[i]);
      dst.first_person = bod2.first_person_slots & (1 << i);
   }

   auto dummy = QModelIndex{};
   auto tl    = this->index(0, 0, dummy);
   auto br    = this->index(slot_count - 1, ColumnCount - 1, dummy);
   emit dataChanged(tl, br);
}
void RaceBipedObjectSlotsModel::commitTo(dovah::loaded_forms::Race& race, dovah::loaded_forms::components::biped_object& bod2) const {
   for (size_t i = 0; i < slot_count; ++i) {
      auto& src = this->_slots[i];
      race.biped_object_info.names[i] = src.name.toStdString();
      {
         auto mask = 1 << i;
         if (src.first_person) {
            bod2.first_person_slots |= mask;
         } else {
            bod2.first_person_slots &= ~mask;
         }
      }
   }
}