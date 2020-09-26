#include "cell_list.h"
#include <QHeaderView>
#include "../../../editor/core.h"
#include "../../../dovah/form_stub.h"

CellListModelItem::CellListModelItem(dovah::form_stub* stub) {
   this->stub     = stub;
   this->editorID = QString::fromUtf8(stub->get_editor_id());
   this->formID   = stub->formID;
   this->gridX    = stub->groupInfo.gridX;
   this->gridY    = stub->groupInfo.gridY;
}
bool CellListModelItem::cellIsLoaded() {
   if (this->stub)
      return this->stub->form != nullptr;
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
QModelIndex CellListModel::index(int row, int column, const QModelIndex& parent) const {
   if (!this->hasIndex(row, column, parent))
      return QModelIndex();
   item_type* childItem = this->root->child(row);
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
   return this->root->childCount();
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
   auto item   = (item_type*)index.internalPointer();
   auto column = index.column();
   switch (role) {
      case Qt::DisplayRole:
         switch (column) {
            case 0: return item->editorID;
            case 1: return QString::asprintf("%08X", item->formID);
            case 2: return item->gridX;
            case 3: return item->gridY;
         }
         break;
      case Qt::DecorationRole:
         if (column == 0) {
            //
            // TODO: icons per form type
            //
         }
         break;
      case Qt::UserRole: // used for sorting
         switch (column) {
            case 0: return item->editorID;
            case 1: return item->formID;
            case 2: return item->gridX;
            case 3: return item->gridY;
         }
         break;
      case Qt::UserRole + 1: // used for filtering
         switch (column) {
            case 0: return item->editorID;
            case 1: return QString::asprintf("%08X", item->formID);
            case 2:
            case 3: return QVariant(); // don't allow filtering by the grid coordinates
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

void CellListModel::insertItem(dovah::form_stub* stub) {
   this->root->_children.push_back(new CellListModelItem(stub));
}
void CellListModel::updateExistingItem(const dovah::form_stub* stub) {
   if (stub->formType != dovah::form_type::cell)
      return;
   auto& list = this->root->_children;
   auto  size = list.size();
   for (size_t i = 0; i < size; ++i) {
      auto* item = list[i];
      if (item->stub == stub) {
         item->update();
         auto index = this->index(i, 0, QModelIndex());
         emit dataChanged(index, index);
         break;
      }
   }
}

void CellListModel::clear() {
   this->beginResetModel();
   this->root->clear();
   this->endResetModel();
}
void CellListModel::rebuild(const dovah::form_stub* worldspace) {
   this->clear();
   //
   auto& editor = DovahKitCore::get();
   if (!editor.has_data())
      return;
   //
   QVector<CellListModelItem*> additions;
   if (worldspace) {
      editor.for_each_form_of_type(dovah::form_type::cell, [worldspace, &additions](dovah::form_stub* stub) {
         if (stub->groupInfo.parentFormID != worldspace->formID)
            return false;
         additions.push_back(new CellListModelItem(stub));
         return false;
      });
   } else {
      editor.for_each_form_of_type(dovah::form_type::cell, [&additions](dovah::form_stub* stub) {
         if (stub->groupInfo.parentFormID)
            return false;
         additions.push_back(new CellListModelItem(stub));
         return false;
      });
   }
   if (!additions.size())
      return;
   auto& list = this->root->_children;
   this->beginInsertRows(QModelIndex(), 0, additions.size() - 1); // we're not passing the count, we're passing the index of the last row. how annoying.
   for (auto* item : additions)
      list.push_back(item);
   this->endInsertRows();
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

   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, &CellList::clear);
   QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) {
      if (auto model = this->unwrappedModel())
         model->updateExistingItem(stub);
   });
   
   QObject::connect(this->selectionModel(), &QItemSelectionModel::selectionChanged, [this](const QItemSelection& selected, const QItemSelection& deselected) {
      const auto* item = this->_getCurrentItem();
      if (!item || !item->stub)
         return;
      emit this->currentCellChanged(item->stub);
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
   return item->stub;
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