#include "cell_ref_list.h"
#include <QHeaderView>
#include <QMetaMethod>
#include <QScrollBar>
#include "helpers/qt/strings.h"
#include "editor/core.h"
#include "editor/helpers/form_identifiers_to_string.h"
#include "editor/helpers/form_type_name_to_string.h"
#include "editor/open_window_for_form.h"
#include "editor/form_stub_meta_type.h"
#include "dovah/form_stub.h"
#include "dovah/form_stubs/helpers/for_each_child_form.h"
#include "dovah/form_stubs/helpers/get_base_form.h"
#include "dovah/utils/form_type_is_cell_child.h"
#include "widgets/DKHeaderView.h"
#include "../../generic/QItemSelectionModelEx.h"

CellRefListModelItem::CellRefListModelItem(const dovah::form_stub* stub) {
   this->stub = stub;
   this->update();
}
dovah::form_type CellRefListModelItem::formType() const noexcept {
   return this->baseType;
}
void CellRefListModelItem::update() {
   auto stub = this->stub;
   this->base         = dovah::form_stub_helpers::get_base_form(*stub);
   this->formID.raw   = stub->formID;
   this->editorID.raw = QString::fromUtf8(stub->get_editor_id());
   if (this->base) {
      this->baseType = this->base->form_type;
      if (this->editorID.raw.isEmpty())
         this->editorID.raw = QString::fromUtf8(this->base->get_editor_id());
   } else {
      switch (stub->form_type) {
         case dovah::form_type::land:
         case dovah::form_type::navmesh:
            this->baseType = stub->form_type;
            if (this->editorID.raw.isEmpty()) {
               this->editorID.raw = editor_helpers::form_type_name_to_string(stub->form_type);
            }
            break;
         default:
            this->baseType = dovah::form_type::none;
      }
   }
   
   this->is_active     = stub->is_edited_or_in_active_file() && !stub->test_record_flags(dovah::tes_file_record_header::flag::partial);
   this->is_injected   = stub->is_injected();
   this->is_persistent = stub->test_record_flags(dovah::tes_file_record_header::flag::persistent);
   
   {
      bool  deleted = stub->is_deleted();
      auto& dst     = this->formID.display;
      dst = editor_helpers::form_id_to_string(this->formID.raw);
      this->formID.filter = dst;
      if (this->is_persistent)
         dst += CellRefListModel::tr("(P)", "form ID marker: persistent forms");
      if (this->is_active || deleted)
         dst += CellRefListModel::tr(" *", "form ID marker: edited or deleted forms");
      if (deleted)
         dst += CellRefListModel::tr(" D", "form ID marker: deleted forms");
   }
   {
      bool  deleted = stub->is_deleted();
      auto& dst     = this->editorID.display;
      dst = this->editorID.raw;
      if (this->is_active || deleted)
         dst += CellRefListModel::tr(" *", "form ID marker: edited or deleted forms");
   }
}

#pragma region CellRefListModel
/*static*/ bool CellRefListModel::acceptsFormType(dovah::form_type ft) {
   return dovah::form_type_is_cell_child(ft);
}

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
   if (!dovah::form_type_is_cell_child(stub->form_type))
      return;
   this->_insertItem(stub, false);
}
void CellRefListModel::formModified(const dovah::form_stub* stub) {
   if (!this->last_used_cell)
      return;
   if (stub->get_parent_form() != this->last_used_cell)
      return;
   if (!acceptsFormType(stub->form_type))
      return;
   auto& list = this->children;
   auto  size = list.size();
   for (size_t i = 0; i < size; ++i) {
      auto* item = list[i];
      if (item->stub == stub) {
         bool was_filtered = false;
         if (this->proxy) {
            was_filtered = !this->proxy->filterAcceptsRow(i, {});
         }
         item->update();
         auto root  = QModelIndex();
         auto start = this->index(i, 0, root);
         auto end   = this->index(i, this->columnCount(root), root);
         emit dataChanged(start, end);
         if (was_filtered) {
            emit this->editedFormNoLongerFiltered(const_cast<dovah::form_stub*>(stub));
         }
         break;
      }
   }
}
void CellRefListModel::formDeletionImminent(const dovah::form_stub* stub, bool is_just_flagged) {
   if (!this->last_used_cell)
      return;
   if (stub->get_parent_form() != this->last_used_cell)
      return;
   if (!dovah::form_type_is_cell_child(stub->form_type))
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

#pragma region Overrides
QVariant CellRefListModel::data(const QModelIndex& index, int role) const {
   if (!index.isValid())
      return QVariant();
   auto item    = (item_type*)index.internalPointer();
   auto column  = index.column();
   bool edited  = item->is_active;
   bool deleted = (item->stub && item->stub->is_deleted());
   switch (role) {
      case Qt::ToolTipRole:
      case Qt::DisplayRole:
         switch (column) {
            case ColumnName:
               return item->editorID.display;
            case ColumnFormID:
               return item->formID.display;
            case ColumnFormType:
               return editor_helpers::form_type_name_to_string(item->baseType);
         }
         break;
      case Qt::DecorationRole:
         if (column == ColumnName) {
            //
            // TODO: icons per form type
            //
         }
         break;
      case Qt::ForegroundRole:
         if (column == ColumnFormID && item->is_injected) // show injected forms' IDs in color
            return QColor::fromRgb(0x309000);
         break;
      case Qt::TextAlignmentRole:
         if (column == ColumnFormType)
            return (Qt::Alignment::Int)(Qt::AlignHCenter);
         break;
      case SortingRole: // used for sorting
         switch (column) {
            case ColumnName:     return item->editorID.raw;
            case ColumnFormID:   return item->formID.raw;
            case ColumnFormType: return editor_helpers::form_type_name_to_string(item->baseType);
         }
         break;
      case FilteringRole: // used for filtering
         switch (column) {
            case ColumnName:     return item->editorID.raw;
            case ColumnFormID:   return item->formID.filter;
            case ColumnFormType: return {};
         }
         break;
      case FormStubRole:
         return QVariant::fromValue(const_cast<dovah::form_stub*>(item->stub));
   }
   return QVariant();
}
Qt::ItemFlags CellRefListModel::flags(const QModelIndex& index) const {
   if (!index.isValid())
      return Qt::NoItemFlags;
   return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
}
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
QModelIndex CellRefListModel::index(int row, int column, const QModelIndex& parent) const {
   if (!this->hasIndex(row, column, parent))
      return QModelIndex();
   const auto* childItem = this->children.value(row);
   if (childItem)
      return this->createIndex(row, column, (void*)childItem);
   return QModelIndex();
}
QModelIndex CellRefListModel::parent(const QModelIndex& index) const {
   return {};
}
int CellRefListModel::rowCount(const QModelIndex& parent) const {
   if (parent.column() > 0)
      return 0;
   return this->children.size();
}
#pragma endregion

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
   dovah::form_stub_helpers::for_each_child_form(*cell, [this](dovah::form_stub& stub) {
      if (!acceptsFormType(stub.form_type))
         return false;
      this->_insertItem(&stub, true);
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
const CellRefListModel::item_type* CellRefListModel::row(int rowIndex) const noexcept {
   return this->children.value(rowIndex);
}

QModelIndex CellRefListModel::index(const form_stub& stub) const {
   const auto& list = this->children;
   for (size_t i = 0; i < list.size(); ++i) {
      const auto& item = list[i];
      if (item->stub == &stub)
         return this->index((int)i, 0, {});
   }
   return {};
}

void CellRefListModel::makeAwareOfProxy(CellRefListModelProxy* proxy) {
   this->proxy = proxy;
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
void CellRefListModelProxy::setFormType(dovah::form_type ft) {
   this->_formType = ft;
   this->invalidateFilter();
}
#pragma endregion

#pragma region CellRefList
CellRefList::CellRefList(QWidget* parent) : QTableView(parent) {
   auto underlying = new model_type;
   auto proxy      = new CellRefListModelProxy(this);
   proxy->setSourceModel(underlying);
   underlying->makeAwareOfProxy(proxy);
   this->setModel(proxy);
   this->verticalHeader()->setDefaultSectionSize(0);
   this->sortByColumn(0, Qt::AscendingOrder);
   this->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
   this->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
   {
      auto* header = new DKHeaderView(Qt::Horizontal, this);
      header->setFlexResizeEnabled(true);
      {
         auto* old = this->horizontalHeader();
         header->setHighlightSections(old->highlightSections());
         header->setSortIndicatorShown(old->isSortIndicatorShown());
         header->setSectionsClickable(old->sectionsClickable());
      }
      this->setHorizontalHeader(header);
      //
      auto metrics = QFontMetrics(this->font());
      header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignBaseline);
      header->setMinimumSectionSize(2);
      header->setColumnFlex(model_type::ColumnName,     1, 0);
      header->setColumnFlex(model_type::ColumnFormID,   0, 0, metrics.boundingRect("00000000").width() * 1.5F + 4);
      header->setColumnFlex(model_type::ColumnFormType, 0, 0, metrics.boundingRect("Weapon").width() * 1.5F + 4);
      header->modSectionSizeTo(model_type::ColumnFormID, 4); // mimics a user resize and shrinks the column
      for(int i = 0; i < model_type::ColumnCount; ++i)
         header->setSectionResizeMode(i, QHeaderView::Interactive);
      header->setStretchLastSection(false);
   }

   //
   // Qt sorting APIs are flaky as hell; we have to do these specific steps 
   // in this specific order.
   // <https://forum.qt.io/topic/94977/default-sort-indicator-order-in-qheaderview/7>
   //
   this->horizontalHeader()->setSortIndicator(0, Qt::AscendingOrder);
   this->setSortingEnabled(true);

   // forward signal to outside world
   QObject::connect(underlying, &model_type::editedFormNoLongerFiltered, this, &CellRefList::editedFormNoLongerFiltered);

   QObject::connect(this, &QTableView::doubleClicked, [this](const QModelIndex& index) {
      if (auto* stub = this->formStub())
         open_edit_dialog_for_form(*stub, this);
   });
   {
      auto* sm = this->selectionModel();
      if (sm)
         sm->deleteLater();
      auto* nsm = new QItemSelectionModelEx(this->model(), this);
      this->setSelectionModel(nsm);
   }
   QObject::connect((QItemSelectionModelEx*)this->selectionModel(), &QItemSelectionModelEx::userInitiatedSelectionChanged, [this](const QItemSelection& selected, const QItemSelection& deselected) {
      static const QMetaMethod _signal = QMetaMethod::fromSignal(&CellRefList::userInitiatedSelectionChanged);
      if (!this->isSignalConnected(_signal))
         //
         // We should only do this work if something is actually connected to the signal we plan on emitting.
         //
         return;

      std::vector<dovah::form_stub*> sel;
      std::vector<dovah::form_stub*> desel;

      auto* model = this->model();

      auto proxy_qmi_list = selected.indexes();
      sel.reserve(proxy_qmi_list.size());
      for (const auto& qmi : proxy_qmi_list) {
         auto* stub = (dovah::form_stub*) model->data(qmi, model_type::FormStubRole).value<void*>();
         if (stub)
            sel.push_back(stub);
      }
      
      proxy_qmi_list = deselected.indexes();
      desel.reserve(proxy_qmi_list.size());
      for (const auto& qmi : proxy_qmi_list) {
         auto* stub = (dovah::form_stub*) model->data(qmi, model_type::FormStubRole).value<void*>();
         if (stub)
            desel.push_back(stub);
      }

      emit this->userInitiatedSelectionChanged(sel, desel);
   });
   QObject::connect(this->selectionModel(), &QItemSelectionModel::selectionChanged, [this](const QItemSelection& selected, const QItemSelection& deselected) {
      static const QMetaMethod _signal = QMetaMethod::fromSignal(&CellRefList::selectionChanged);
      if (!this->isSignalConnected(_signal))
         //
         // We should only do this work if something is actually connected to the signal we plan on emitting.
         //
         return;

      std::vector<dovah::form_stub*> sel;
      std::vector<dovah::form_stub*> desel;

      auto* model = this->model();

      auto proxy_qmi_list = selected.indexes();
      sel.reserve(proxy_qmi_list.size());
      for (const auto& qmi : proxy_qmi_list) {
         auto* stub = (dovah::form_stub*) model->data(qmi, model_type::FormStubRole).value<void*>();
         if (stub)
            sel.push_back(stub);
      }
      
      proxy_qmi_list = deselected.indexes();
      desel.reserve(proxy_qmi_list.size());
      for (const auto& qmi : proxy_qmi_list) {
         auto* stub = (dovah::form_stub*) model->data(qmi, model_type::FormStubRole).value<void*>();
         if (stub)
            desel.push_back(stub);
      }

      emit this->selectionChanged(sel, desel);
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
   emit this->filterChanged();
}
void CellRefList::selectStub(const dovah::form_stub* stub, QItemSelectionModel::SelectionFlags flags) {
   auto* sm = this->selectionModel();
   if (!sm)
      return;
   if (!stub) {
      if (flags & QItemSelectionModel::SelectionFlag::Clear) {
         sm->clear();
      }
      return;
   }
   auto inner_qmi = this->unwrappedModel()->index(*stub);
   auto proxy_qmi = this->proxyModel()->mapFromSource(inner_qmi);
   sm->select(QItemSelection(proxy_qmi, proxy_qmi.siblingAtColumn(model_type::ColumnCount - 1)), flags);
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
void CellRefList::setFormTypeFilter(dovah::form_type ft) {
   auto* proxy = (proxy_type*)this->model();
   if (!proxy)
      return;
   proxy->setFormType(ft);
   emit this->filterChanged();
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
std::vector<dovah::form_stub*> CellRefList::formStubs() const {
   std::vector<dovah::form_stub*> out;

   auto* model  = this->model();
   int   rows   = model->rowCount();
   out.reserve(rows);
   for (int i = 0; i < rows; ++i) {
      auto  qmi  = model->index(i, 0);
      auto* stub = (dovah::form_stub*) model->data(qmi, model_type::FormStubRole).value<void*>();
      if (stub)
         out.push_back(stub);
   }
   return out;
}
std::vector<dovah::form_stub*> CellRefList::selectedStubs() const {
   std::vector<dovah::form_stub*> out;

   auto* model  = this->model();
   auto  select = this->selectionModel()->selection().indexes();
   if (select.size() <= 0)
      return out;

   out.reserve(select.size());
   for (const auto& qmi : select) {
      auto* stub = (dovah::form_stub*) model->data(qmi, model_type::FormStubRole).value<void*>();
      if (stub)
         out.push_back(stub);
   }
   return out;
}

void CellRefList::rebuildModel() {
   auto m = this->unwrappedModel();
   if (!m)
      return;
   const dovah::form_stub* stub = nullptr;
   if (this->_cellSelector) {
      stub = this->_cellSelector->formStub();
      if (stub && stub->form_type != dovah::form_type::cell)
         stub = nullptr;
   }
   m->rebuild(stub);
   //
   // Scroll to the top when the list is completely rebuilt. Qt does not 
   // have this behavior built in even when you reset the model.
   //
   if (auto* scrollbar = this->verticalScrollBar())
      scrollbar->setValue(0);
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