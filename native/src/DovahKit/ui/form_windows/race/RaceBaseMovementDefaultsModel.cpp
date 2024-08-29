#include "./RaceBaseMovementDefaultsModel.h"
#include "dovah/forms/Race.h"
#include "dovah/form_stub.h"
#include "editor/core.h"

RaceBaseMovementDefaultsModel::RaceBaseMovementDefaultsModel(QObject* parent) : QAbstractItemModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() {
      this->_slots = {};
   });
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub) {
      for (size_t i = 0; i < this->_slots.size(); ++i) {
         auto& item = this->_slots[i];
         if (item.stub != stub)
            continue;
         item = {};

         auto qmi = this->index(i, 0, {});
         emit dataChanged(qmi, qmi);
      }
   });
   QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) {
      for (size_t i = 0; i < this->_slots.size(); ++i) {
         auto& item = this->_slots[i];
         if (item.stub != stub)
            continue;
         item.cached_editor_id = QString::fromStdString(stub->editorID);

         auto qmi = this->index(i, 0, {});
         emit dataChanged(qmi, qmi);
      }
   });
}

#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex RaceBaseMovementDefaultsModel::index(int row, int col, const QModelIndex& parent) const /*override*/ {
         if (row < 0 || row >= this->_slots.size())
            return {};
         if (col < 0 || col >= this->columnCount())
            return {};
         if (parent.isValid())
            return {};
         return this->createIndex(row, col, nullptr);
      }
      /*virtual*/ QModelIndex RaceBaseMovementDefaultsModel::parent(const QModelIndex& index) const /*override*/ {
         return {};
      }
      /*virtual*/ QModelIndex RaceBaseMovementDefaultsModel::sibling(int row, int column, const QModelIndex& index) const /*override*/ {
         if (!index.isValid())
            return {};
         return this->index(row, column, {});
      }
      /*virtual*/ int RaceBaseMovementDefaultsModel::rowCount(const QModelIndex& parent) const /*override*/ {
         return this->_slots.size();
      }
      /*virtual*/ int RaceBaseMovementDefaultsModel::columnCount(const QModelIndex& parent) const /*override*/ {
         return ColumnCount;
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant RaceBaseMovementDefaultsModel::data(const QModelIndex& index, int role) const /*override*/ {
         if (!index.isValid())
            return {};
         if (index.row() >= this->_slots.size())
            return {};
         auto& src = this->_slots[index.row()];
         switch (index.column()) {
            case Column::Type:
               if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
                  switch ((MoveType)index.row()) {
                     case MoveType::walk:
                        return tr("Walk");
                     case MoveType::run:
                        return tr("Run");
                     case MoveType::swim:
                        return tr("Swim");
                     case MoveType::sneak:
                        return tr("Sneak");
                     case MoveType::sprint:
                        return tr("Sprint");
                     case MoveType::fly:
                        return tr("Fly");
                  }
               }
               return {};
            case Column::Form:
               return src.cached_editor_id;
         }
         return {};
      }
      /*virtual*/ Qt::ItemFlags RaceBaseMovementDefaultsModel::flags(const QModelIndex& index) const /*override*/ {
         if (!index.isValid()) {
            return {};
         }
         auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
         return flags;
      }
   #pragma endregion
      /*virtual*/ QVariant RaceBaseMovementDefaultsModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
         if (orientation != Qt::Orientation::Horizontal)
            return {};
         if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
            return {};
         switch (section) {
            case Column::Type:
               return tr("Move Type");
            case Column::Form:
               return tr("Editor ID");
         }
         return {};
      }
#pragma endregion

void RaceBaseMovementDefaultsModel::initializeFrom(const dovah::loaded_forms::Race& race) {
   this->beginResetModel();
   for (size_t i = 0; i < num_move_types; ++i) {
      auto& dst = this->_slots[i];
      dst.stub = race.movement.types.list[i].get_form_stub();
      dst.cached_editor_id = QString::fromStdString(dst.stub->editorID);
   }
   this->endResetModel();
}
void RaceBaseMovementDefaultsModel::commitTo(dovah::loaded_forms::Race& race) const {
   for (size_t i = 0; i < num_move_types; ++i) {
      auto& src = this->_slots[i];
      race.movement.types.list[i].set(race, src.stub);
   }
}

dovah::form_stub* RaceBaseMovementDefaultsModel::form(size_t row) const {
   if (row >= this->_slots.size())
      return nullptr;
   return this->_slots[row].stub;
}
void RaceBaseMovementDefaultsModel::setForm(size_t row, dovah::form_stub* stub) {
   if (row >= this->_slots.size())
      return;
   auto& dst = this->_slots[row];
   if (dst.stub == stub)
      return;
   dst.stub = stub;

   auto qmi = this->index(row, 0, {});
   emit dataChanged(qmi, qmi, {});
}