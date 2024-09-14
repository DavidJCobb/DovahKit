#include "delete_form_table.h"
#include <QHeaderView>
#include <QLineEdit>
#include "../../../helpers/qt/strings.h"
#include "../../../editor/core.h"
#include "../../../editor/helpers/form_identifiers_to_string.h"
#include "../../../editor/open_window_for_form.h"

#pragma region DeleteFormDialogListModel
DeleteFormDialogListModelItem::DeleteFormDialogListModelItem(const dovah::form_stub& stub) : stub(stub) {
   this->updateFromStub();
}
void DeleteFormDialogListModelItem::updateFromStub() {
   this->formID   = stub.formID;
   this->editorID = QString::fromStdString(stub.editorID);
   //
   uint32_t signature = dovah::form_type_info::lookup(stub.form_type).signature;
   this->signature = cobb::qt::four_cc_to_string(signature);
   //
   if (dovah::form_type_is_reference(stub.form_type)) {
      auto parent = stub.get_parent_form();
      assert(parent->form_type == dovah::form_type::cell && "When this code was written, it was only possible for refs to appear inside of CELLs. Looks like something's changed?");
      if (parent) {
         auto name = parent->get_editor_id();
         auto id   = editor_helpers::form_id_to_string(parent->formID);
         this->parentCell = QString("[CELL:%1]").arg(id);
         if (name && name[0]) {
            this->parentCell += name;
         } else {
            auto world = parent->get_parent_form();
            assert(world->form_type == dovah::form_type::worldspace && "When this code was written, it was only possible for CELLs to appear inside of WRLDs. Looks like something's changed?");
            if (world) {
               auto    id   = editor_helpers::form_id_to_string(world->formID);
               QString s    = QString("[WRLD:%1]%2").arg(id).arg(world->get_editor_id());
               int32_t x;
               int32_t y;
               QString grid;
               if (parent->get_grid_coordinates(x, y)) {
                  grid = QString("(%1, %2)").arg(x).arg(y);
               } else {
                  grid = QString("(?, ?)");
               }
               this->parentCell = QString("%2%3 in %1").arg(s).arg(this->parentCell).arg(grid);
            }
         }
      }
   }
}

DeleteFormDialogListModel::DeleteFormDialogListModel(QObject* parent) : QAbstractTableModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::formModified,             this, &DeleteFormDialogListModel::formModified);
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent,     this, &DeleteFormDialogListModel::formDeletionImminent);
   QObject::connect(&editor, &DovahKitCore::formRenumbered,           this, &DeleteFormDialogListModel::formRenumbered);
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent,      this, &DeleteFormDialogListModel::clear);
}

void DeleteFormDialogListModel::formModified(const dovah::form_stub* stub) {
   auto& list = this->children;
   auto  size = list.size();
   for (int i = 0; i < size; ++i) {
      auto* item = list[i];
      if (&item->stub == stub) {
         item->updateFromStub();
         auto root  = QModelIndex();
         auto start = this->index(i, 0, root);
         auto end   = this->index(i, this->columnCount(root), root);
         emit dataChanged(start, end);
         return;
      }
   }
}
void DeleteFormDialogListModel::formDeletionImminent(const dovah::form_stub* stub, bool is_just_flagged) {
   this->remove(*stub);
}
void DeleteFormDialogListModel::formRenumbered(const dovah::form_stub* stub, dovah::bare_form_id_t oldID, dovah::bare_form_id_t newID) {
   auto& list = this->children;
   auto  size = list.size();
   for (size_t i = 0; i < size; ++i) {
      auto* item = list[i];
      if (&item->stub == stub) {
         item->updateFromStub();
         auto index = this->index(i, 1, QModelIndex());
         emit dataChanged(index, index);
         return;
      }
   }
}
//
QModelIndex DeleteFormDialogListModel::index(int row, int column, const QModelIndex& parent) const {
   if (!this->hasIndex(row, column, parent))
      return QModelIndex();
   item_type* childItem = this->children.value(row);
   if (childItem)
      return this->createIndex(row, column, childItem);
   return QModelIndex();
}
QModelIndex DeleteFormDialogListModel::parent(const QModelIndex& index) const {
   return QModelIndex();
}
int DeleteFormDialogListModel::rowCount(const QModelIndex& parent) const {
   if (parent.column() > 0)
      return 0;
   return this->children.size();
}
int DeleteFormDialogListModel::columnCount(const QModelIndex& item) const {
   return 3;
}
Qt::ItemFlags DeleteFormDialogListModel::flags(const QModelIndex& index) const {
   if (!index.isValid())
      return Qt::NoItemFlags;
   return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled;
}
QVariant DeleteFormDialogListModel::data(const QModelIndex& index, int role) const {
   if (!index.isValid())
      return QVariant();
   auto item   = (item_type*)index.internalPointer();
   auto column = index.column();
   switch (column) {
      case 0: // signature
         switch (role) {
            case Qt::DisplayRole:
            case Qt::UserRole + 0: // sorting
               return item->signature;
         }
         break;
      case 1: // form ID
         switch (role) {
            case Qt::DisplayRole:
               return QString("%1").arg(item->formID, 8, 16, QChar('0')).toUpper();
               return QString("%1").arg(item->formID, 8, 16, QChar('0')).toUpper();
            case Qt::UserRole + 0: // sorting
               return item->formID;
         }
         break;
      case 2: // editor ID or parent cell information
         switch (role) {
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
            case Qt::UserRole + 0: // sorting
               if (item->editorID.isEmpty() && !item->parentCell.isEmpty())
                  return tr("unnamed; placed in %1").arg(item->parentCell);
               return item->editorID;
         }
         break;
   }
   return QVariant();
}
inline const DeleteFormDialogListModel::item_type* DeleteFormDialogListModel::row(int rowIndex) const noexcept {
   return this->children.value(rowIndex);
}
//
QVariant DeleteFormDialogListModel::headerData(int section, Qt::Orientation orientation, int role) const {
   if (orientation != Qt::Orientation::Horizontal)
      return QVariant();
   switch (role) {
      case Qt::DisplayRole:
         switch (section) {
            case 0: return tr("Type",    "use info report");
            case 1: return tr("Form ID", "use info report");
            case 2: return tr("Editor ID / Details", "use info report");
         }
         break;
   }
   return QVariant();
}
bool DeleteFormDialogListModel::contains(const dovah::form_stub& stub) const noexcept {
   for (auto* item : this->children)
      if (&item->stub == &stub)
         return true;
   for (auto* item : this->queued_additions)
      if (&item->stub == &stub)
         return true;
   return false;
}

void DeleteFormDialogListModel::clear() {
   this->beginResetModel();
   for (auto* item : this->children)
      delete item;
   this->children.clear();
   for (auto* item : this->queued_additions)
      delete item;
   this->queued_additions.clear();
   this->endResetModel();
}
void DeleteFormDialogListModel::insert(const dovah::form_stub& stub, bool queued) {
   auto item = new item_type(stub);
   if (queued) {
      this->queued_additions.push_back(item);
   } else {
      auto first_inserted = this->children.size();
      auto last_inserted  = first_inserted;
      this->beginInsertRows(QModelIndex(), first_inserted, last_inserted); // we're not passing the count, we're passing the index of the last row. how annoying.
      this->children.push_back(item);
      this->endInsertRows();
   }
}
void DeleteFormDialogListModel::remove(const dovah::form_stub& stub) {
   auto& list = this->children;
   auto  size = list.size();
   for (int i = 0; i < size; ++i) {
      auto* item = list[i];
      if (&item->stub == &stub) {
         this->beginRemoveRows(QModelIndex(), i, i);
         list.removeAt(i);
         this->endRemoveRows();
         return;
      }
   }
}
void DeleteFormDialogListModel::commitQueuedAdditions() {
   auto& from = this->queued_additions;
   auto& list = this->children;
   if (from.isEmpty())
      return;
   int first = list.size();
   int last  = first + from.size() - 1;
   list.reserve(last + 1);
   this->beginInsertRows(QModelIndex(), first, last);
   for (auto* item : from)
      list.push_back(item);
   from.clear();
   this->endInsertRows();
}
//
void DeleteFormDialogListModel::insertDeletions(const dovah::form_deletion_request& request) {
   auto list = request.get_forms_pending_delete();
   for (auto* stub : list) {
      this->insert(*stub, true);
   }
   this->commitQueuedAdditions();
}
void DeleteFormDialogListModel::insertUsers(const dovah::form_deletion_request& request) {
   const auto list = request.get_forms_pending_delete();

   auto _will_be_deleted = [&list](const dovah::form_stub& stub) {
      for (auto* s : list)
         if (s == &stub)
            return true;
      return false;
   };

   for (const auto* stub : list) {
      for (const auto& pair : stub->inbound) {
         const auto& entry = pair.second;
         const auto& stub  = *entry.other;
         {  // Don't show child forms unless they have a use of the to-be-deleted form besides childhood
            auto* parent = stub.get_parent_form();
            if (parent) {
               if (_will_be_deleted(*parent) && entry.refcount == 1) {
                  continue;
               }
            }
         }
         if (_will_be_deleted(stub))
            continue;
         if (this->contains(stub))
            continue;
         this->insert(stub, true);
      }
   }
   this->commitQueuedAdditions();
}
#pragma endregion

DeleteFormDialogListModelProxy::DeleteFormDialogListModelProxy(QObject* parent) : QSortFilterProxyModel(parent) {
   this->setFilterCaseSensitivity(Qt::CaseInsensitive);
   this->setFilterRole(Qt::UserRole + 1);
   this->setFilterKeyColumn(-1);
   this->setSortCaseSensitivity(Qt::CaseInsensitive);
   this->setSortRole(Qt::UserRole + 0);
}

#pragma region DeleteFormDialogList
DeleteFormDialogList::DeleteFormDialogList(QWidget* parent) : QTableView(parent) {
   {
      auto model = new model_type(this);
      auto proxy = new DeleteFormDialogListModelProxy(this);
      proxy->setSourceModel(model);
      this->setModel(proxy);
   }
   this->verticalHeader()->setDefaultSectionSize(0);
   this->sortByColumn(0, Qt::AscendingOrder);
   //
   auto header  = this->horizontalHeader();
   auto metrics = QFontMetrics(this->font());
   header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignBaseline);
   header->setMinimumSectionSize(2);
   header->resizeSection(0, metrics.boundingRect("XMMX").width() * 1.5F + 4);
   header->resizeSection(1, metrics.boundingRect("00000000").width() * 1.5F + 4);
   header->setSectionResizeMode(0, QHeaderView::Interactive);
   header->setSectionResizeMode(1, QHeaderView::Interactive);
   header->setSectionResizeMode(2, QHeaderView::Stretch);
};
#pragma endregion