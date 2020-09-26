#include "cell_ref_list.h"
#include <QHeaderView>
#include "../../../helpers/qt/strings.h"
#include "../../../editor/core.h"
#include "../../../dovah/form_stub.h"
#include "../../../dovah/form_stub_helpers.h"

CellRefListModelItem::CellRefListModelItem(dovah::form_stub* stub) {
   this->stub     = stub;
   this->base     = dovah::form_stub_helpers::get_base_form(stub);
   this->editorID = QString::fromUtf8(stub->get_editor_id());
   if (this->editorID.isEmpty() && this->base)
      this->editorID = QString::fromUtf8(this->base->get_editor_id());
   this->formID   = stub->formID;
}
dovah::form_type_t CellRefListModelItem::formType() const noexcept {
   if (this->base)
      return this->base->formType;
   return dovah::form_type::none;
}
void CellRefListModelItem::update() {
   auto stub = this->stub;
   this->base = dovah::form_stub_helpers::get_base_form(stub);
   //
   this->editorID = QString::fromUtf8(stub->get_editor_id());
   if (this->editorID.isEmpty() && this->base)
      this->editorID = QString::fromUtf8(this->base->get_editor_id());
   //
   this->formID = stub->formID;
}

#pragma region CellRefListModel
QModelIndex CellRefListModel::index(int row, int column, const QModelIndex& parent) const {
   if (!this->hasIndex(row, column, parent))
      return QModelIndex();
   item_type* childItem = this->root->child(row);
   if (childItem)
      return this->createIndex(row, column, childItem);
   return QModelIndex();
}
QModelIndex CellRefListModel::parent(const QModelIndex& index) const {
   return QModelIndex();
}
int CellRefListModel::rowCount(const QModelIndex& parent) const {
   if (parent.column() > 0)
      return 0;
   return this->root->childCount();
}
int CellRefListModel::columnCount(const QModelIndex& item) const {
   return 3;
}
Qt::ItemFlags CellRefListModel::flags(const QModelIndex& index) const {
   if (!index.isValid())
      return Qt::NoItemFlags;
   return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
}
QVariant CellRefListModel::data(const QModelIndex& index, int role) const {
   if (!index.isValid())
      return QVariant();
   auto item   = (item_type*)index.internalPointer();
   auto column = index.column();
   switch (role) {
      case Qt::DisplayRole:
         switch (column) {
            case 0: return item->editorID;
            case 1: return QString::asprintf("%08X", item->formID);
            case 2: return cobb::qt::four_cc_to_string( dovah::form_type_info::lookup(item->formType()).signature );
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
            case 2: return item->formType();
         }
         break;
      case Qt::UserRole + 1: // used for filtering
         switch (column) {
            case 0: return item->editorID;
            case 1: return QString::asprintf("%08X", item->formID);
            case 2: return QVariant();
         }
         break;
   }
   return QVariant();
}
//
QVariant CellRefListModel::headerData(int section, Qt::Orientation orientation, int role) const {
   if (orientation != Qt::Orientation::Horizontal)
      return QVariant();
   switch (role) {
      case Qt::DisplayRole:
         switch (section) {
            case 0: return tr("Editor ID", "cell view ref list");
            case 1: return tr("Form ID",   "cell view ref list");
            case 2: return tr("Type",      "cell view ref list");
         }
         break;
   }
   return QVariant();
}

void CellRefListModel::insertItem(dovah::form_stub* stub) {
   this->root->_children.push_back(new CellRefListModelItem(stub));
}
void CellRefListModel::updateExistingItem(const dovah::form_stub* stub) {
   if (!this->last_used_cell)
      return;
   if (stub->groupInfo.parentFormID != this->last_used_cell->formID)
      return;
   if (!dovah::form_type_info::form_type_is_reference(stub->formType))
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

void CellRefListModel::clear() {
   this->beginResetModel();
   this->root->clear();
   this->last_used_cell = nullptr;
   this->endResetModel();
}
void CellRefListModel::rebuild(const dovah::form_stub* cell) {
   this->clear();
   if (!cell)
      return;
   //
   auto& editor = DovahKitCore::get();
   if (!editor.has_data())
      return;
   //
   this->last_used_cell = cell;
   QVector<CellRefListModelItem*> additions;
   dovah::form_stub_helpers::for_each_child_form(cell, [&additions](dovah::form_stub* stub) {
      if (!dovah::form_type_info::form_type_is_reference(stub->formType))
         return false;
      additions.push_back(new CellRefListModelItem(stub));
      return false;
   });
   if (!additions.size())
      return;
   auto& list = this->root->_children;
   this->beginInsertRows(QModelIndex(), 0, additions.size() - 1); // we're not passing the count, we're passing the index of the last row. how annoying.
   for (auto* item : additions)
      list.push_back(item);
   this->endInsertRows();
}
#pragma endregion

#pragma region CellRefList
CellRefList::CellRefList(QWidget* parent) : QTableView(parent) {
   auto underlying = new model_type;
   auto proxy      = new CellRefListModelProxy(this);
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
   header->resizeSection(2, metrics.boundingRect("XXXX").width() * 1.5F + 4);
   header->setSectionResizeMode(0, QHeaderView::Stretch);
   header->setSectionResizeMode(1, QHeaderView::Interactive);
   header->setSectionResizeMode(2, QHeaderView::Interactive);

   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, &CellRefList::clear);
   QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) {
      if (auto model = this->unwrappedModel())
         model->updateExistingItem(stub);
   });
   
   //
   // TODO: context menu for right-clicking refs
   //
   QObject::connect(this, &QTableView::doubleClicked, [this](const QModelIndex& index) {
      auto data = this->_getCurrentItem();
      if (!data || !data->stub)
         return;
      open_edit_dialog_for_form(data->stub, this);
   });
};
void CellRefList::setCellPicker(const CellList* picker) {
   if (picker == this->_cellSelector)
      return;
   if (this->_cellSelector)
      QObject::disconnect(this->_cellSelector, nullptr, this, nullptr);
   this->_cellSelector = picker;
   if (picker) {
      QObject::connect(picker, &CellList::currentCellChanged, this, &CellRefList::rebuildModel);
   }
}
void CellRefList::rebuildModel() {
   auto m = this->unwrappedModel();
   if (!m)
      return;
   const dovah::form_stub* stub = nullptr;
   if (this->_cellSelector) {
      stub = this->_cellSelector->formStub();
      if (stub && stub->formType != dovah::form_type::cell)
         stub = nullptr;
   }
   m->rebuild(stub);
}
void CellRefList::clear() {
   auto m = this->unwrappedModel();
   if (!m)
      return;
   m->clear();
}
CellRefList::model_item_type* CellRefList::_getCurrentItem() const noexcept {
   auto proxy = (proxy_type*)this->model();
   auto select = this->selectionModel()->selection().indexes();
   if (select.size() <= 0)
      return nullptr;
   auto& idx = proxy->mapToSource(select[0]);
   return (model_item_type*)idx.internalPointer();
}
#pragma endregion