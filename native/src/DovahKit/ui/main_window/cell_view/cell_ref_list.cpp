#include "cell_ref_list.h"
#include <QHeaderView>
#include "../../../helpers/qt/strings.h"
#include "../../../editor/core.h"
#include "../../../editor/helpers/form_identifiers_to_string.h"
#include "../../../editor/open_window_for_form.h"
#include "../../../dovah/form_stub.h"
#include "../../../dovah/form_stub_helpers.h"

CellRefListModelItem::CellRefListModelItem(const dovah::form_stub* stub) {
   this->stub = stub;
   this->update();
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
   //
   this->is_active   = stub->is_edited_or_in_active_file() && !stub->test_record_flags(dovah::tes_file_record_header::flag::partial);
   this->is_injected = stub->is_injected();
}

#pragma region CellRefListModel
CellRefListModel::CellRefListModel(QObject* parent) : QAbstractTableModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent,  this, &CellRefListModel::clear);
   QObject::connect(&editor, &DovahKitCore::formCreated,          this, &CellRefListModel::formCreated);
   QObject::connect(&editor, &DovahKitCore::formModified,         this, &CellRefListModel::formModified);
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, &CellRefListModel::formDeletionImminent);
   QObject::connect(&editor, &DovahKitCore::formRenumbered,       this, &CellRefListModel::formRenumbered);
   QObject::connect(&editor, &DovahKitCore::formsRenumberedEnMasse, this, [this]() { this->rebuild(this->last_used_cell); });
}
void CellRefListModel::formCreated(const dovah::form_stub* stub) {
   if (!this->last_used_cell)
      return;
   if (stub->get_parent_form() != this->last_used_cell)
      return;
   if (!dovah::form_type_info::form_type_is_reference(stub->formType))
      return;
   this->_insertItem(stub, false);
}
void CellRefListModel::formModified(const dovah::form_stub* stub) {
   if (!this->last_used_cell)
      return;
   if (stub->get_parent_form() != this->last_used_cell)
      return;
   if (!dovah::form_type_info::form_type_is_reference(stub->formType))
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
void CellRefListModel::formDeletionImminent(const dovah::form_stub* stub, bool is_just_flagged) {
   if (!this->last_used_cell)
      return;
   if (stub->get_parent_form() != this->last_used_cell)
      return;
   if (!dovah::form_type_info::form_type_is_reference(stub->formType))
      return;
   auto& list = this->children;
   auto  size = list.size();
   for (size_t i = 0; i < size; ++i) {
      auto* item = list[i];
      if (item->stub == stub) {
         if (is_just_flagged) {
            auto index = this->index(i, 0, QModelIndex());
            emit dataChanged(index, index);
            break;
         }
         this->beginRemoveRows(QModelIndex(), i, i);
         list.remove(i);
         this->endRemoveRows();
         break;
      }
   }
}
void CellRefListModel::formRenumbered(const dovah::form_stub* stub, dovah::bare_form_id_t oldID, dovah::bare_form_id_t newID) {
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

void CellRefListModel::_insertItem(const form_stub* stub, bool queued) {
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

QModelIndex CellRefListModel::index(int row, int column, const QModelIndex& parent) const {
   if (!this->hasIndex(row, column, parent))
      return QModelIndex();
   item_type* childItem = this->children.value(row);
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
   return this->children.size();
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
   auto item    = (item_type*)index.internalPointer();
   auto column  = index.column();
   bool edited  = item->is_active;
   bool deleted = (item->stub && item->stub->is_deleted());
   switch (role) {
      case Qt::DisplayRole:
         switch (column) {
            case 0:
               return tr("%1%2")
                  .arg(item->editorID)
                  .arg((edited || deleted) ? tr(" * ", "edited form editor ID marker") : "");
            case 1:
               return editor_helpers::form_id_to_string(item->formID) + ((edited || deleted) ? tr(" * ", "edited form ID marker") : "") + (deleted ? tr("D", "deleted form ID marker") : "");
            case 2:
               return editor_helpers::form_signature_to_string(item->stub);
         }
         break;
      case Qt::DecorationRole:
         if (column == 0) {
            //
            // TODO: icons per form type
            //
         }
         break;
      case Qt::ForegroundRole:
         if (column == 1 && item->is_injected) // show injected forms' IDs in color
            return QColor::fromRgb(0x309000);
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
            case 1: return editor_helpers::form_id_to_string(item->formID);
            case 2: return QVariant();
         }
         break;
   }
   return QVariant();
}
inline const CellRefListModel::item_type* CellRefListModel::row(int rowIndex) const noexcept {
   return this->children.value(rowIndex);
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
   this->_insertItem(stub, false);
}

void CellRefListModel::clear() {
   this->beginResetModel();
   for (auto* item : this->children)
      delete item;
   this->children.clear();
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
   dovah::form_stub_helpers::for_each_child_form(cell, [this](dovah::form_stub* stub) {
      if (!dovah::form_type_info::form_type_is_reference(stub->formType))
         return false;
      this->_insertItem(stub, true);
      return false;
   });
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

#pragma region CellRefListModelProxy
CellRefListModelProxy::CellRefListModelProxy(QObject* parent) : QSortFilterProxyModel(parent) {
   this->setFilterCaseSensitivity(Qt::CaseInsensitive);
   this->setFilterRole(Qt::UserRole + 1);
   this->setFilterKeyColumn(-1);
   this->setSortCaseSensitivity(Qt::CaseInsensitive);
   this->setSortRole(Qt::UserRole + 0);
}
bool CellRefListModelProxy::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const {
   if (this->_formType != dovah::form_type::none) {
      auto* model = (model_type*)this->sourceModel();
      auto* item  = model->row(sourceRow);
      if (item && item->formType() != this->_formType) {
         return false;
      }
   }
   return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
}
void CellRefListModelProxy::setFormType(dovah::form_type_t ft) {
   this->_formType = ft;
   this->invalidateFilter();
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
   header->resizeSection(2, metrics.boundingRect("XMMX").width() * 1.5F + 4);
   header->setSectionResizeMode(0, QHeaderView::Stretch);
   header->setSectionResizeMode(1, QHeaderView::Interactive);
   header->setSectionResizeMode(2, QHeaderView::Interactive);

   QObject::connect(this, &QTableView::doubleClicked, [this](const QModelIndex& index) {
      auto stub = this->formStub();
      if (!stub)
         return;
      open_edit_dialog_for_form(stub, this);
   });
   
   QObject::connect(this->_filterThrottle, &QTimer::timeout, [this]() {
      if (this->_filter)
         this->refilterModelByText(this->_filter->text());
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
void CellRefList::refilterModelByText(const QString& text) {
   auto wrapper = (QSortFilterProxyModel*)this->model();
   if (!wrapper)
      return;
   wrapper->setFilterFixedString(text);
}
void CellRefList::textFilterChanged() {
   auto& timer = *this->_filterThrottle;
   if (timer.isActive())
      return;
   timer.start(200);
}
void CellRefList::textFilterFinished() {
   this->_filterThrottle->stop();
   if (this->_filter)
      this->refilterModelByText(this->_filter->text());
}
void CellRefList::setTextFilter(QLineEdit* field) {
   this->_filterThrottle->stop();
   if (this->_filter) {
      QObject::disconnect(this->_filter, &QLineEdit::textEdited,      this, &CellRefList::textFilterChanged);
      QObject::disconnect(this->_filter, &QLineEdit::editingFinished, this, &CellRefList::textFilterFinished);
   }
   this->_filter = field;
   if (!field)
      return;
   this->refilterModelByText(field->text());
   QObject::connect(field, &QLineEdit::textEdited,      this, &CellRefList::textFilterChanged);
   QObject::connect(field, &QLineEdit::editingFinished, this, &CellRefList::textFilterFinished);
}
void CellRefList::setFormTypeFilter(dovah::form_type_t ft) {
   auto* proxy = (proxy_type*)this->model();
   if (!proxy)
      return;
   proxy->setFormType(ft);
}
dovah::bare_form_id_t CellRefList::formID() const noexcept {
   const auto* item = this->_getCurrentItem();
   if (!item || !item->stub)
      return 0;
   return item->stub->formID;
}
dovah::form_stub* CellRefList::formStub() const noexcept {
   const auto* item = this->_getCurrentItem();
   if (!item || !item->stub)
      return nullptr;
   return const_cast<dovah::form_stub*>(item->stub);
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
   auto proxy  = (proxy_type*)this->model();
   auto select = this->selectionModel()->selection().indexes();
   if (select.size() <= 0)
      return nullptr;
   auto idx = proxy->mapToSource(select[0]);
   return (model_item_type*)idx.internalPointer();
}
#pragma endregion