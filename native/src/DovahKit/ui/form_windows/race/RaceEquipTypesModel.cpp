#include "./RaceEquipTypesModel.h"
#include "dovah/data/weapon_type.h"
#include "dovah/forms/Race.h"

RaceEquipTypesModel::RaceEquipTypesModel(QObject* parent) : QAbstractItemModel(parent) {
}
   
#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex RaceEquipTypesModel::index(int row, int col, const QModelIndex& parent) const /*override*/ {
         if (row < 0 || row >= this->_data.size())
            return {};
         if (col < 0 || col >= this->columnCount())
            return {};
         if (parent.isValid())
            return {};
         return this->createIndex(row, col, nullptr);
      }
      /*virtual*/ QModelIndex RaceEquipTypesModel::parent(const QModelIndex& index) const /*override*/ {
         return {};
      }
      /*virtual*/ QModelIndex RaceEquipTypesModel::sibling(int row, int column, const QModelIndex& index) const /*override*/ {
         if (!index.isValid())
            return {};
         return this->index(row, column, {});
      }
      /*virtual*/ int RaceEquipTypesModel::rowCount(const QModelIndex& parent) const /*override*/ {
         return this->_data.size();
      }
      /*virtual*/ int RaceEquipTypesModel::columnCount(const QModelIndex& parent) const /*override*/ {
         return ColumnCount;
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant RaceEquipTypesModel::data(const QModelIndex& index, int role) const /*override*/ {
         if (!index.isValid())
            return {};
         if (index.row() >= this->_data.size())
            return {};
         switch (role) {
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
               switch ((dovah::weapon_type)index.row()) {
                  case dovah::weapon_type::hand_to_hand_melee:
                     return tr("Hand-To-Hand Melee");
                  case dovah::weapon_type::one_hand_axe:
                     return tr("One-Handed Axe");
                  case dovah::weapon_type::one_hand_dagger:
                     return tr("One-Handed Dagger");
                  case dovah::weapon_type::one_hand_mace:
                     return tr("One-Handed Mace");
                  case dovah::weapon_type::one_hand_sword:
                     return tr("One-Handed Sword");
                  case dovah::weapon_type::two_hand_axe:
                     return tr("Two-Handed Axe");
                  case dovah::weapon_type::two_hand_sword:
                     return tr("Two-Handed Sword");
                  case dovah::weapon_type::staff:
                     return tr("Staff");
                  case dovah::weapon_type::bow:
                     return tr("Bow");
                  case dovah::weapon_type::crossbow:
                     return tr("Crossbow");
               }
               switch (index.row()) {
                  case 10:
                     return tr("Spell");
                  case 11:
                     return tr("Shield");
                  case 12:
                     return tr("Torch");
               }
               break;
            case Qt::CheckStateRole:
               return this->_data[index.row()].enabled ? Qt::CheckState::Checked : Qt::CheckState::Unchecked;
         }
         return {};
      }
      /*virtual*/ Qt::ItemFlags RaceEquipTypesModel::flags(const QModelIndex& index) const /*override*/ {
         if (!index.isValid()) {
            return {};
         }
         auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
         flags |= Qt::ItemFlag::ItemIsUserCheckable | Qt::ItemFlag::ItemIsEditable;
         return flags;
      }
      #pragma region Write-access
         /*virtual*/ bool RaceEquipTypesModel::setData(const QModelIndex& index, const QVariant& value, int role) /*override*/ {
            if (!index.isValid() || index.row() >= this->_data.size() || index.column() >= ColumnCount)
               return false;
            if (role != Qt::CheckStateRole)
               return false;

            this->_data[index.row()].enabled = (value.toInt() == Qt::CheckState::Checked) ? true : false;
            emit dataChanged(index, index, { role });
            return true;
         }
      #pragma endregion
   #pragma endregion
      /*virtual*/ QVariant RaceEquipTypesModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
         if (orientation != Qt::Orientation::Horizontal)
            return {};
         if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
            return {};
         return tr("Equip Types");
      }
#pragma endregion

void RaceEquipTypesModel::initializeFrom(const dovah::loaded_forms::Race& race) {
   this->beginResetModel();
   for (size_t i = 0; i < this->_data.size(); ++i) {
      bool enabled = race.equipment.flags & (1 << i);
      this->_data[i].enabled = enabled;
   }
   this->endResetModel();
}
void RaceEquipTypesModel::commitTo(dovah::loaded_forms::Race& race) const {
   auto& dst = race.equipment.flags;
   for (size_t i = 0; i < this->_data.size(); ++i) {
      std::decay_t<decltype(dst)> mask = 1 << i;
      if (this->_data[i].enabled)
         dst |= mask;
      else
         dst &= ~mask;
   }
}