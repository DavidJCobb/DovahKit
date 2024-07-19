#include "./DKLeveledListPreviewModel.h"
#include "dovah/forms/components/leveled_list.h"
#include "dovah/form_stub.h"
#include "editor/core.h"

namespace {
   static const QVector<int> typical_roles_to_notify_changes_for = { Qt::DisplayRole, Qt::ToolTipRole };
}

DKLeveledListPreviewModel::DKLeveledListPreviewModel(QObject* parent) : QAbstractItemModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() {
      this->_clear();
   });
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub, bool just_being_flagged) {
      auto&  list = this->_items;
      size_t size = list.size();
      for (size_t i = 0; i < size; ++i) {
         auto& item = list[i];
         if (item.form == stub) {
            this->beginRemoveRows({}, i, i);
            list.erase(list.begin() + i);
            --i;
            --size;
            this->endRemoveRows();
            continue;
         }
         if (item.owner == stub) {
            item.owner = nullptr;
            item.cached.ownerEditorID.clear();
            
            auto qmi = this->index(i, Column::Owner, {});
            emit dataChanged(qmi, qmi, typical_roles_to_notify_changes_for);
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
         auto& item = this->_items[i];
         if (stub == item.form || stub == item.owner) {
            start = std::numeric_limits<size_t>::max();
            end   = 0;

            if (stub == item.form) {
               item.cached.editorID = QString::fromStdString(stub->editorID);
               _update_column_range(Column::Form);
            }
            if (item.owner == stub) {
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
DKLeveledListPreviewModel::~DKLeveledListPreviewModel() {
   this->_clear();
}
      
#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex DKLeveledListPreviewModel::index(int row, int column, const QModelIndex& parent) const /*override*/ {
         if (row < 0 || column < 0)
            return {};
         if (column >= this->columnCount())
            return {};
         if (row >= this->rowCount())
            return {};
         if (parent.isValid()) // no nesting
            return {};
         return this->createIndex(row, column, nullptr);
      }
      /*virtual*/ QModelIndex DKLeveledListPreviewModel::parent(const QModelIndex& index) const /*override*/ {
         return {};
      }
      /*virtual*/ QModelIndex DKLeveledListPreviewModel::sibling(int row, int column, const QModelIndex& index) const /*override*/ {
         if (row < 0 || column < 0)
            return {};
         if (column >= this->columnCount())
            return {};
         if (row >= this->rowCount())
            return {};
         if (!index.isValid() || !index.internalPointer())
            return {};
         return this->createIndex(row, column, nullptr);
      }
      /*virtual*/ int DKLeveledListPreviewModel::rowCount(const QModelIndex& parent) const /*override final*/ {
         return this->_items.size();
      }
      /*virtual*/ int DKLeveledListPreviewModel::columnCount(const QModelIndex& item) const /*override final*/ {
         if (this->showsContainerItemFields())
            return Column::_COUNT_IF_ITEMS;
         return Column::_COUNT_ALWAYS;
      }

      /*virtual*/ QVariant DKLeveledListPreviewModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
         if (orientation != Qt::Orientation::Horizontal)
            return {};
         if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
            return {};
         switch (section) {
            case Column::Count:
               return tr("Count", "column header");
            case Column::Form:
               return tr("Form", "column header");
            case Column::Owner:
               return tr("Owner", "column header");
            case Column::Health:
               return tr("Health", "column header");
         }
         return {};
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant DKLeveledListPreviewModel::data(const QModelIndex& index, int role) const /*override*/ {
         if (!index.isValid())
            return {};
         if (index.row() >= this->_items.size())
            return {};
         const auto& item   = this->_items[index.row()];
         const auto  column = index.column();
         switch (role) {
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
               switch (column) {
                  case Column::Count:
                     return item.count;
                  case Column::Form:
                     return item.cached.editorID;
                  case Column::Owner:
                     return item.cached.ownerEditorID;
                  case Column::Health:
                     return item.health * health_display_mult;
               }
               break;
            case Qt::TextAlignmentRole:
               switch (column) {
                  case Column::Count:
                  case Column::Health:
                     return (int)(Qt::AlignRight | Qt::AlignVCenter);
               }
               break;
         }
         return {};
      }
      /*virtual*/ Qt::ItemFlags DKLeveledListPreviewModel::flags(const QModelIndex& index) const /*override*/ {
         if (!index.isValid())
            return {};
         return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemNeverHasChildren;
      }
   #pragma endregion
#pragma endregion

void DKLeveledListPreviewModel::overwrite(const std::vector<dovah::leveled_list_preview::entry>& src_list) {
   this->beginResetModel();

   this->_items.clear();
   this->_items.reserve(src_list.size());
   for (const auto& src : src_list) {

      // Strip out illegal entries.
      if (!src.form || !src.count)
         continue;

      auto& dst = this->_items.emplace_back();
      dst.count = src.count;
      dst.form  = src.form;
      if (auto& src_coed_opt = src.extra; src_coed_opt.has_value()) {
         auto& src_coed = src_coed_opt.value();
         dst.health = src_coed.health;
         dst.owner  = src_coed.owner;
      }

      if (auto* stub = dst.form)
         dst.cached.editorID = QString::fromStdString(stub->editorID);
      if (auto* stub = dst.owner)
         dst.cached.ownerEditorID = QString::fromStdString(stub->editorID);
   }

   this->endResetModel();
}

void DKLeveledListPreviewModel::setShowsContainerItemFields(bool v) {
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

void DKLeveledListPreviewModel::_clear() {
   this->beginResetModel();
   this->_items.clear();
   this->endResetModel();
}