#include "cell_list.h"
#include <QHeaderView>
#include "../../../editor/core.h"
#include "../../../dovah/form_stub.h"

CellListModelItem::CellListModelItem(const dovah::form_stub* stub) {
   this->stub     = stub;
   this->editorID = QString::fromUtf8(stub->get_editor_id());
   this->formID   = stub->formID;
   this->gridX    = stub->groupInfo.gridX;
   this->gridY    = stub->groupInfo.gridY;
}
bool CellListModelItem::cellIsLoaded() {
   //
   // TODO: It's not enough to check if the form is loaded, because it could be loaded by a 
   // properties dialog or something. What we want to know is if the Render Window has a 
   // cell loaded and ready to render.
   //
   return false;
}
void CellListModelItem::update() {
   auto stub = this->stub;
   this->editorID = QString::fromUtf8(stub->get_editor_id());
   this->formID   = stub->formID;
   this->gridX    = stub->groupInfo.gridX;
   this->gridY    = stub->groupInfo.gridY;
}

#pragma region CellListModel
CellListModel::CellListModel(QObject* parent) : QAbstractTableModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent,  this, &CellListModel::clear);
   QObject::connect(&editor, &DovahKitCore::formCreated,          this, &CellListModel::formCreated);
   QObject::connect(&editor, &DovahKitCore::formModified,         this, &CellListModel::formModified);
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, &CellListModel::formDeletionImminent);
   QObject::connect(&editor, &DovahKitCore::formRenumbered,       this, &CellListModel::formRenumbered);
   QObject::connect(&editor, &DovahKitCore::formsRenumberedEnMasse, this, [this]() { this->rebuild(this->worldspace); });
}

void CellListModel::formCreated(const dovah::form_stub* stub) {
   if (stub->formType != dovah::form_type::cell)
      return;
   if (this->worldspace) {
      if (stub->groupInfo.parentFormID != this->worldspace->formID)
         return;
   } else {
      if (stub->groupInfo.parentFormID)
         return;
   }
   this->insertItem(stub, false);
}
void CellListModel::formModified(const dovah::form_stub* stub) {
   if (stub->formType != dovah::form_type::cell)
      return;
   auto& list = this->children;
   auto  size = list.size();
   for (size_t i = 0; i < size; ++i) {
      auto* item = list[i];
      if (item->stub == stub) {
         item->update();
         auto root  = QModelIndex();
         auto start = this->index(i, 0, root);
         auto end   = this->index(i, this->columnCount(root), root);
         emit dataChanged(start, end);
         break;
      }
   }
}
void CellListModel::formDeletionImminent(const dovah::form_stub* stub, bool is_just_flagged) {
   if (stub->formType != dovah::form_type::cell)
      return;
   auto& list = this->children;
   auto  size = list.size();
   for (size_t i = 0; i < size; ++i) {
      auto* item = list[i];
      if (item->stub == stub) {
         if (is_just_flagged) {
            //
            // TODO: Do we want to even display cells that were flagged as deleted?
            //
         }
         this->beginRemoveRows(QModelIndex(), i, i);
         list.remove(i);
         this->endRemoveRows();
         break;
      }
   }
}
void CellListModel::formRenumbered(const dovah::form_stub* stub, dovah::bare_form_id_t oldID, dovah::bare_form_id_t newID) {
   auto& list = this->children;
   auto  size = list.size();
   for (size_t i = 0; i < size; ++i) {
      auto* item = list[i];
      if (item->stub == stub) {
         item->update();
         auto index = this->index(i, 1, QModelIndex());
         emit dataChanged(index, index);
         return;
      }
   }
}

QModelIndex CellListModel::index(int row, int column, const QModelIndex& parent) const {
   if (!this->hasIndex(row, column, parent))
      return QModelIndex();
   item_type* childItem = this->children.value(row);
   if (childItem)
      return this->createIndex(row, column, childItem);
   return QModelIndex();
}
QModelIndex CellListModel::parent(const QModelIndex& index) const {
   return QModelIndex();
}
int CellListModel::rowCount(const QModelIndex& parent) const {
   if (parent.column() > 0)
      return 0;
   return this->children.size();
}
int CellListModel::columnCount(const QModelIndex& item) const {
   return 4;
}
Qt::ItemFlags CellListModel::flags(const QModelIndex& index) const {
   if (!index.isValid())
      return Qt::NoItemFlags;
   return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
}
QVariant CellListModel::data(const QModelIndex& index, int role) const {
   if (!index.isValid())
      return QVariant();
   auto item    = (item_type*)index.internalPointer();
   auto column  = index.column();
   bool edited  = (item->stub && item->stub->is_edited());
   bool deleted = (item->stub && item->stub->is_deleted());
   switch (column) {
      case 0: // editor ID
         switch (role) {
            case Qt::DisplayRole:
            case SortingRole:
            case FilteringRole:
               {
                  QString text = item->editorID;
                  if (text.isEmpty())
                     text = tr("Unnamed Cell", "cell view cell list");
                  if (deleted && role == Qt::DisplayRole)
                     text += tr(" * ", "edited form ID marker");
                  return text;
               }
            case SortOverrideRole:
               if (item->editorID.isEmpty())
                  return 1;
            case Qt::FontRole:
               if (item->editorID.isEmpty()) {
                  auto font = QFont();
                  font.setItalic(true);
                  return font;
               }
               break;
         }
         break;
      case 1: // form ID
         switch (role) {
            case Qt::DisplayRole:
               return QString::asprintf("%08X", item->formID) + ((edited || deleted) ? tr(" * ", "edited form ID marker") : "") + (deleted ? tr("D", "deleted form ID marker") : "");
            case FilteringRole:
               return QString::asprintf("%08X", item->formID);
            case SortingRole:
               return item->formID;
         }
         break;
      case 2: // grid X
         switch (role) {
            case Qt::DisplayRole:
            case SortingRole:
               return item->gridX;
            case FilteringRole:
               return QVariant(); // don't allow filtering by the grid coordinates
         }
         break;
      case 3: // grid Y
         switch (role) {
            case Qt::DisplayRole:
            case SortingRole:
               return item->gridY;
            case FilteringRole:
               return QVariant(); // don't allow filtering by the grid coordinates
         }
         break;
   }
   return QVariant();
}
//
QVariant CellListModel::headerData(int section, Qt::Orientation orientation, int role) const {
   if (orientation != Qt::Orientation::Horizontal)
      return QVariant();
   switch (role) {
      case Qt::DisplayRole:
         switch (section) {
            case 0: return tr("Editor ID", "cell view cell list");
            case 1: return tr("Form ID",   "cell view cell list");
            case 2: return tr("X", "cell view cell list");
            case 3: return tr("Y", "cell view cell list");
         }
         break;
   }
   return QVariant();
}

void CellListModel::insertItem(const dovah::form_stub* stub, bool queued) {
   if (!stub)
      return;
   auto item = new item_type(stub);
   if (queued) {
      this->queued_additions.push_back(item);
   } else {
      auto i = this->children.size();
      this->beginInsertRows(QModelIndex(), i, i);
      this->children.push_back(item);
      this->endInsertRows();
   }
}

void CellListModel::clear() {
   this->beginResetModel();
   this->worldspace = nullptr;
   for (auto* item : this->children)
      delete item;
   this->children.clear();
   this->endResetModel();
}
void CellListModel::rebuild(const dovah::form_stub* worldspace) {
   this->clear();
   //
   auto& editor = DovahKitCore::get();
   if (!editor.has_data())
      return;
   //
   this->worldspace = worldspace;
   if (worldspace) {
      editor.for_each_form_of_type(dovah::form_type::cell, [worldspace, this](dovah::form_stub* stub) {
         if (stub->groupInfo.parentFormID != worldspace->formID)
            return false;
         this->insertItem(stub, true);
         return false;
      });
   } else {
      editor.for_each_form_of_type(dovah::form_type::cell, [this](dovah::form_stub* stub) {
         if (stub->groupInfo.parentFormID)
            return false;
         this->insertItem(stub, true);
         return false;
      });
   }
   auto& queue = this->queued_additions;
   auto  count = queue.size();
   if (!count)
      return;
   auto& list  = this->children;
   auto  first = list.size();
   auto  last  = first + (count - 1);
   this->beginInsertRows(QModelIndex(), first, last);
   for (auto* item : queue)
      list.push_back(item);
   queue.clear();
   this->endInsertRows();
}
#pragma endregion

#pragma region CellListModelProxy
CellListModelProxy::CellListModelProxy(QObject* parent) : QSortFilterProxyModel(parent) {
   this->setFilterCaseSensitivity(Qt::CaseInsensitive);
   this->setFilterRole(Qt::UserRole + 1);
   this->setFilterKeyColumn(-1);
   this->setSortCaseSensitivity(Qt::CaseInsensitive);
   this->setSortRole(Qt::UserRole);
   //
   this->setSortOverrideRole((Qt::ItemDataRole)CellListModel::SortOverrideRole);
}
bool CellListModelProxy::lessThan(const QModelIndex& left, const QModelIndex& right) const {
   auto source    = this->sourceModel();
   auto sort_role = this->sortRole();
   //
   {
      auto override_role = this->_sortOverrideRole;
      if (override_role != Qt::DisplayRole) {
         auto a = source->data(left,  override_role).toInt();
         auto b = source->data(right, override_role).toInt();
         //
         // The override role can be used to force an item to the start of the list, or to the end. 
         // If both items are trying to force to the same side of the list, then they should be 
         // sorted normally; otherwise, the override should be honored.
         //
         if ((a | b) && (a * b <= 0)) { // at least one is non-zero; signs are different or one is zero
            if (a > 0)
               return false;
            if (a < 0)
               return true;
            //
            if (b > 0)
               return true;
            if (b < 0)
               return false;
         }
      }
   }
   //
   QVariant leftData  = source->data(left,  sort_role);
   QVariant rightData = source->data(right, sort_role);
   return QString::localeAwareCompare(leftData.toString(), rightData.toString()) < 0;
}
void CellListModelProxy::setSortOverrideRole(Qt::ItemDataRole r) {
   if (this->_sortOverrideRole == r)
      return;
   this->_sortOverrideRole = r;
   this->sort(this->sortColumn());
}
#pragma endregion

#pragma region CellList
CellList::CellList(QWidget* parent) : QTableView(parent) {
   auto underlying = new model_type;
   auto proxy      = new CellListModelProxy(this);
   proxy->setSourceModel(underlying);
   this->setModel(proxy);
   this->verticalHeader()->setDefaultSectionSize(0);
   this->sortByColumn(0, Qt::AscendingOrder);
   this->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
   //
   auto header  = this->horizontalHeader();
   auto metrics = QFontMetrics(this->font());
   header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignBaseline);
   header->setMinimumSectionSize(2);
   header->resizeSection(1, metrics.boundingRect("00000000").width() * 1.5F + 4);
   header->resizeSection(2, metrics.boundingRect("000").width() * 1.5F + 4);
   header->resizeSection(3, metrics.boundingRect("000").width() * 1.5F + 4);
   header->setSectionResizeMode(0, QHeaderView::Stretch);
   header->setSectionResizeMode(1, QHeaderView::Interactive);
   header->setSectionResizeMode(2, QHeaderView::Interactive);
   header->setSectionResizeMode(3, QHeaderView::Interactive);
   
   QObject::connect(this->selectionModel(), &QItemSelectionModel::selectionChanged, [this](const QItemSelection& selected, const QItemSelection& deselected) {
      if (auto* stub = this->formStub())
         emit this->currentCellChanged(stub);
   });
};
void CellList::setWorldspacePicker(const FormsOfTypeCombobox* picker) {
   if (picker == this->_worldspaceSelector)
      return;
   if (this->_worldspaceSelector)
      QObject::disconnect(this->_worldspaceSelector, nullptr, this, nullptr);
   this->_worldspaceSelector = picker;
   if (picker) {
      QObject::connect(picker, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CellList::rebuildModel);
   }
}
dovah::bare_form_id_t CellList::formID() const noexcept {
   const auto* item = this->_getCurrentItem();
   if (!item || !item->stub)
      return 0;
   return item->stub->formID;
}
dovah::form_stub* CellList::formStub() const noexcept {
   const auto* item = this->_getCurrentItem();
   if (!item || !item->stub)
      return nullptr;
   return const_cast<dovah::form_stub*>(item->stub);
}
void CellList::rebuildModel() {
   auto m = this->unwrappedModel();
   if (!m)
      return;
   const dovah::form_stub* stub = nullptr;
   if (this->_worldspaceSelector) {
      auto formID = this->_worldspaceSelector->formID();
      if (formID) {
         auto& editor = DovahKitCore::get();
         stub = editor.get_form(formID);
         if (stub->formType != dovah::form_type::worldspace)
            stub = nullptr;
      }
   }
   m->rebuild(stub);
   //
   auto header = this->horizontalHeader();
   header->setSectionHidden(2, !stub); // don't show grid coordinates for interior cells, as they do not have any
   header->setSectionHidden(3, !stub); // 
}
void CellList::clear() {
   auto m = this->unwrappedModel();
   if (!m)
      return;
   m->clear();
}
CellList::model_item_type* CellList::_getCurrentItem() const noexcept {
   auto proxy  = (proxy_type*)this->model();
   auto select = this->selectionModel()->selection().indexes();
   if (select.size() <= 0)
      return nullptr;
   auto& idx = proxy->mapToSource(select[0]);
   return (model_item_type*)idx.internalPointer();
}
#pragma endregion