#include "form_table.h"
#include <QHeaderView>
#include "basic_form_type_treeview.h"
#include "../../../editor/core.h"
#include "../../../dovah/form_stub.h"

#pragma region FormTableModel
FormTableModelItem::FormTableModelItem(dovah::form_stub* stub) {
   this->editorID  = QString::fromUtf8(stub->get_editor_id());
   this->formID    = stub->formID;
   this->userCount = stub->inbound.size();
}
QModelIndex FormTableModel::index(int row, int column, const QModelIndex& parent) const {
   if (!this->hasIndex(row, column, parent))
      return QModelIndex();
   item_type* childItem = this->root->child(row);
   if (childItem)
      return this->createIndex(row, column, childItem);
   return QModelIndex();
}
QModelIndex FormTableModel::parent(const QModelIndex& index) const {
   return QModelIndex();
}
int FormTableModel::rowCount(const QModelIndex& parent) const {
   if (parent.column() > 0)
      return 0;
   return this->root->childCount();
}
int FormTableModel::columnCount(const QModelIndex& item) const {
   return 3;
}
Qt::ItemFlags FormTableModel::flags(const QModelIndex& index) const {
   if (!index.isValid())
      return Qt::NoItemFlags;
   return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
}
QVariant FormTableModel::data(const QModelIndex& index, int role) const {
   if (!index.isValid())
      return QVariant();
   auto item = static_cast<item_type*>(index.internalPointer());
   if (role != Qt::DisplayRole)
      return QVariant();
   switch (index.column()) {
      case 0:
         return item->name();
      case 1:
         return QString::asprintf("%08X", item->formID);
      case 2:
         return item->userCount;
   }
   return QVariant();
}
//
QVariant FormTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
   if (orientation != Qt::Orientation::Horizontal)
      return QVariant();
   if (role == Qt::DisplayRole) {
      switch (section) {
         case 0: return tr("Editor ID", "object window form table");
         case 1: return tr("Form ID",   "object window form table");
         case 2: return tr("Users",     "object window form table");
      }
   }
   return QVariant();
}
void FormTableModel::sort(int column, Qt::SortOrder order) {
   auto signal_parents = QList<QPersistentModelIndex>();
   emit layoutAboutToBeChanged(signal_parents, QAbstractItemModel::VerticalSortHint);
   //
   auto& list = this->root->_children;
   switch (column) {
      case 0:
         std::sort(list.begin(), list.end(), [order](const FormTableModelItem* a, const FormTableModelItem* b) {
            if (order == Qt::DescendingOrder)
               std::swap(a, b);
            auto compare = a->editorID.compare(b->editorID, Qt::CaseInsensitive);
            if (compare)
               return compare < 0;
            return a->formID < b->formID;
         });
         break;
      case 1:
         std::sort(list.begin(), list.end(), [order](const FormTableModelItem* a, const FormTableModelItem* b) {
            if (order == Qt::DescendingOrder)
               std::swap(a, b);
            return a->formID < b->formID;
         });
         break;
      case 2:
         std::sort(list.begin(), list.end(), [order](const FormTableModelItem* a, const FormTableModelItem* b) {
            if (order == Qt::DescendingOrder)
               std::swap(a, b);
            if (a->userCount == b->userCount)
               return a->formID < b->formID;
            return a->userCount < b->userCount;
         });
         break;
   }
   this->last_sort_column = column;
   this->last_sort_order  = order;
   //
   emit layoutChanged(signal_parents, QAbstractItemModel::VerticalSortHint);
}

void FormTableModel::insertItem(dovah::form_stub* stub) {
   this->root->_children.push_back(new FormTableModelItem(stub));
}

void FormTableModel::clear() {
   this->beginResetModel();
   this->root->clear();
   this->endResetModel();
}
#pragma endregion

#pragma region FormTable
FormTable::FormTable(QWidget* parent) : QTableView(parent) {
   this->setModel(new model_type);
   this->verticalHeader()->setDefaultSectionSize(0);
   //
   auto header  = this->horizontalHeader();
   auto metrics = QFontMetrics(this->font());
   header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignBaseline);
   header->setMinimumSectionSize(2);
   header->resizeSection(1, metrics.boundingRect("00000000").width() * 1.5F + 4);
   header->resizeSection(2, metrics.boundingRect("000000").width() * 1.5F + 4);
   header->setSectionResizeMode(0, QHeaderView::Stretch);
   header->setSectionResizeMode(1, QHeaderView::Interactive);
   header->setSectionResizeMode(2, QHeaderView::Interactive);

   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, &FormTable::rebuildModel);
   QObject::connect(&editor, &DovahKitCore::dataAcquireComplete, this, &FormTable::rebuildModel);
}
void FormTable::recheckFormTypes() {
   if (!this->_source)
      return;
   form_type_set next;
   this->_source->getSelectedFormTypes(next);
   if (this->_currentFormTypes == next)
      return;
   this->_currentFormTypes = next;
   this->rebuildModel();
}
void FormTable::rebuildModel() {
   auto m = (model_type*)this->model();
   if (!m)
      return;
   m->clear();
   //
   auto& editor = DovahKitCore::get();
   if (!editor.has_data())
      return;
   //
   if (this->_currentFormTypes.size()) {
      for (auto ft : this->_currentFormTypes)
         editor.for_each_form_of_type(ft, [m](dovah::form_stub* stub) { m->insertItem(stub); return false; });
      //
      auto col = m->lastSortColumn();
      if (col >= 0)
         m->sort(col, m->lastSortOrder());
   }
}
void FormTable::setSource(BasicFormTypeTree* tree) {
   if (this->_source)
      QObject::disconnect(this->_source->selectionModel(), &QItemSelectionModel::selectionChanged , this, &FormTable::_adaptSourceSelectionChange);
   this->_source = tree;
   if (!tree)
      return;
   tree->getSelectedFormTypes(this->_currentFormTypes);
   QObject::connect(tree->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FormTable::_adaptSourceSelectionChange);
}
void FormTable::_adaptSourceSelectionChange(const QItemSelection& selected, const QItemSelection& deselected) {

   this->recheckFormTypes();
}
#pragma endregion