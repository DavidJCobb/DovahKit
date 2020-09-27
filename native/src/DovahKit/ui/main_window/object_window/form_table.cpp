#include "form_table.h"
#include <QHeaderView>
#include <QLineEdit>
#include "basic_form_type_treeview.h"
#include "../../../editor/core.h"
#include "../../../dovah/form_stub.h"

FormTableModelItem::FormTableModelItem(dovah::form_stub* stub) {
   this->stub      = stub;
   this->editorID  = QString::fromUtf8(stub->get_editor_id());
   this->formID    = stub->formID;
   this->userCount = stub->inbound.size();
}
void FormTableModelItem::update() {
   auto stub = this->stub;
   this->editorID  = QString::fromUtf8(stub->get_editor_id());
   this->formID    = stub->formID;
   this->userCount = stub->inbound.size();
}
bool FormTableModelItem::updateUserCount() {
   if (auto* stub = this->stub) {
      auto updated = stub->inbound.size();
      if (this->userCount == updated)
         return false;
      this->userCount = updated;
      return true;
   }
   return false;
}

#pragma region FormTableModel
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
   auto item   = (item_type*)index.internalPointer();
   auto column = index.column();
   switch (role) {
      case Qt::DisplayRole:
         switch (column) {
            case 0: return item->name();
            case 1: return QString::asprintf("%08X", item->formID);
            case 2: return item->userCount;
         }
         break;
      case Qt::DecorationRole:
         if (column == 0) {
            //
            // TODO: icons per form type
            //
         }
         break;
      case Qt::UserRole:
         switch (column) {
            case 0: return item->name();
            case 1: return item->formID;
            case 2: return item->userCount;
         }
         break;
      case Qt::UserRole + 1: // used for filtering
         switch (column) {
            case 0: return item->name();
            case 1: return QString::asprintf("%08X", item->formID);
            case 2: return QVariant(); // don't allow filtering by the use count
         }
         break;
   }
   return QVariant();
}
//
QVariant FormTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
   if (orientation != Qt::Orientation::Horizontal)
      return QVariant();
   switch (role) {
      case Qt::DisplayRole:
         switch (section) {
            case 0: return tr("Editor ID", "object window form table");
            case 1: return tr("Form ID",   "object window form table");
            case 2: return tr("Users",     "object window form table");
         }
         break;
   }
   return QVariant();
}

void FormTableModel::insertItem(dovah::form_stub* stub) {
   this->root->_children.push_back(new FormTableModelItem(stub));
}
void FormTableModel::updateExistingItem(const dovah::form_stub* stub) {
   QVector<dovah::form_stub*> used;
   for (auto& pair : stub->outbound) {
      auto* other = pair.second.other;
      if (other)
         used.push_back(other);
   }
   //
   auto  parent_index = QModelIndex();
   auto& list = this->root->_children;
   auto  size = list.size();
   for (size_t i = 0; i < size; ++i) {
      auto* item = list[i];
      if (item->stub == stub) {
         item->update();
         auto index = this->index(i, 0, parent_index);
         emit dataChanged(index, index);
         break;
      } else if (used.contains(item->stub)) {
         //
         // Update any other forms that need their use counts used because (stub) was 
         // changed to use them.
         //
         if (item->updateUserCount()) {
            auto index = this->index(i, 0, parent_index);
            emit dataChanged(index, index);
         }
      }
   }
}

void FormTableModel::clear() {
   this->beginResetModel();
   this->root->clear();
   this->forms_pending_use_info_update.clear();
   this->endResetModel();
}
void FormTableModel::rebuild(const form_type_set& types) {
   this->clear();
   //
   if (!types.size())
      return;
   auto& editor = DovahKitCore::get();
   if (!editor.has_data())
      return;
   //
   uint32_t total = 0;
   for (auto ft : types)
      total += editor.count_forms_of_type(ft);
   if (!total)
      return;
   this->beginInsertRows(QModelIndex(), 0, total - 1); // we're not passing the count, we're passing the index of the last row. how annoying.
   for (auto ft : types)
      editor.for_each_form_of_type(ft, [this](dovah::form_stub* stub) { this->insertItem(stub); return false; });
   this->endInsertRows();
}
void FormTableModel::prepToUpdateUseInfo(const dovah::form_stub* user) {
   //
   // The (user) form is about to be changed, and those changes may result in it no longer 
   // using some other form. We need to take note of all of the forms that it currently 
   // uses, so that we can update them after the (user) form is changed.
   //
   auto& list = this->forms_pending_use_info_update;
   for (auto& pair : user->outbound) {
      auto stub = pair.second.other;
      if (!stub)
         continue;
      if (!list.contains(stub))
         list.push_back(stub);
   }
}
void FormTableModel::doUseInfoUpdate() {
   auto  parent_index = QModelIndex();
   auto& list = this->root->_children;
   auto  size = list.size();
   for (size_t i = 0; i < size; ++i) {
      auto* item = list[i];
      auto* stub = item->stub;
      if (this->forms_pending_use_info_update.contains(stub)) {
         if (item->updateUserCount()) {
            auto index = this->index(i, 0, parent_index);
            emit dataChanged(index, index);
         }
      }
   }
   this->forms_pending_use_info_update.clear();
}
#pragma endregion

#pragma region FormTable
FormTable::FormTable(QWidget* parent) : QTableView(parent) {
   auto underlying = new model_type;
   auto proxy      = new FormTableModelProxy(this);
   proxy->setSourceModel(underlying);
   this->setModel(proxy);
   this->verticalHeader()->setDefaultSectionSize(0);
   this->sortByColumn(0, Qt::AscendingOrder);
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
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, &FormTable::clear);
   QObject::connect(&editor, &DovahKitCore::dataAcquireComplete, this, &FormTable::rebuildModel);
   QObject::connect(&editor, &DovahKitCore::formModificationImminent, this, [this](dovah::form_stub* user) {
      if (auto model = this->unwrappedModel())
         model->prepToUpdateUseInfo(user);
   });
   QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) {
      if (auto model = this->unwrappedModel()) {
         model->updateExistingItem(stub);
         model->doUseInfoUpdate();
      }
   });

   QObject::connect(this->_filterThrottle, &QTimer::timeout, [this]() {
      if (this->_filter)
         this->refilterModel(this->_filter->text());
   });
};
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
   auto m = this->unwrappedModel();
   if (!m)
      return;
   m->rebuild(this->_currentFormTypes);
}
void FormTable::refilterModel(const QString& text) {
   auto wrapper = (QSortFilterProxyModel*)this->model();
   if (!wrapper)
      return;
   wrapper->setFilterFixedString(text);
}
void FormTable::filterChanged() {
   auto& timer = *this->_filterThrottle;
   if (timer.isActive())
      return;
   timer.start(200);
}
void FormTable::filterFinished() {
   this->_filterThrottle->stop();
   if (this->_filter)
      this->refilterModel(this->_filter->text());
}
void FormTable::clear() {
   auto m = this->unwrappedModel();
   if (!m)
      return;
   m->clear();
}
void FormTable::setFilter(QLineEdit* field) {
   this->_filterThrottle->stop();
   if (this->_filter) {
      QObject::disconnect(this->_filter, &QLineEdit::textEdited, this, &FormTable::filterChanged);
      QObject::disconnect(this->_filter, &QLineEdit::editingFinished, this, &FormTable::filterFinished);
   }
   this->_filter = field;
   if (!field)
      return;
   this->refilterModel(field->text());
   QObject::connect(field, &QLineEdit::textEdited, this, &FormTable::filterChanged);
   QObject::connect(field, &QLineEdit::editingFinished, this, &FormTable::filterFinished);
}
void FormTable::setSource(BasicFormTypeTree* tree) {
   if (this->_source)
      QObject::disconnect(this->_source->selectionModel(), &QItemSelectionModel::selectionChanged , this, &FormTable::recheckFormTypes);
   this->_source = tree;
   if (!tree)
      return;
   tree->getSelectedFormTypes(this->_currentFormTypes);
   QObject::connect(tree->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FormTable::recheckFormTypes);
}
#pragma endregion