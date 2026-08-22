#include "use_info_list.h"
#include <QHeaderView>
#include <QLineEdit>
#include "helpers/qt/strings.h"
#include "dovah/use_info/entry_flag_to_mask.h"
#include "dovah/use_info/entry_flags/base.h"
#include "dovah/use_info/entry_flags/reference.h"
#include "editor/core.h"
#include "editor/helpers/form_identifiers_to_string.h"
#include "editor/open_window_for_form.h"
#include "editor/subsystems/worldedit/core.h"

namespace {
   QString _format_cell(dovah::form_stub* cell) {
      assert(cell->form_type == dovah::form_type::cell);
      //
      auto name = cell->get_editor_id();
      QString cell_text = editor_helpers::form_identifiers_to_string(cell);
      if (name && name[0]) {
         return cell_text;
      }
      auto world = cell->get_parent_form();
      if (world) {
         assert(world->form_type == dovah::form_type::worldspace && "When this code was written, it was only possible for CELLs to appear inside of WRLDs. Looks like something's changed?");
         QString s = editor_helpers::form_identifiers_to_string(world);
         int32_t x;
         int32_t y;
         QString grid;
         if (cell->get_grid_coordinates(x, y)) {
            grid = QString("(%1, %2)").arg(x).arg(y);
         } else {
            grid = QString("(?, ?)");
         }
         return QString("%2%3 in %1").arg(s).arg(cell_text).arg(grid);
      }
      return cell_text;
   }
}

#pragma region FormUseInfoListModel
FormUseInfoListModelItem::FormUseInfoListModelItem(const dovah::use_info::entry* source) {
   bool is_reference = false;
   if (source->other)
      is_reference = dovah::form_type_is_reference(source->other->form_type) && source->flags & dovah::use_info::entry_flag_to_mask(dovah::use_info::entry_flags::reference::base_form);
   //
   this->flags       = source->flags;
   this->countUsed   = source->refcount;
   this->countPlaced = 0;
   if (is_reference) {
      --this->countUsed;
      ++this->countPlaced;
   }
   if (auto stub = source->other) {
      this->otherStub = stub;
      this->updateFromStub();
   }
}
void FormUseInfoListModelItem::updateFromStub() {
   bool is_reference = false;
   if (this->otherStub)
      is_reference = dovah::form_type_is_reference(this->otherStub->form_type) && this->flags & dovah::use_info::entry_flag_to_mask(dovah::use_info::entry_flags::reference::base_form);
   auto stub = this->otherStub;
   //
   this->otherID   = stub->formID;
   this->otherType = stub->form_type;
   this->editorID  = stub->get_editor_id();
   this->signature = editor_helpers::form_signature_to_string(stub);
   //
   if (is_reference) {
      auto parent = stub->get_parent_form();
      if (parent) { // can be nullptr for PlayerRef
         assert(parent->form_type == dovah::form_type::cell && "When this code was written, it was only possible for refs to appear inside of CELLs. Looks like something's changed?");
         this->parentCell = _format_cell(parent);
      }
   } else {
      dovah::form_stub* cell = nullptr;
      if (stub->form_type == dovah::form_type::land) {
         cell = stub->get_parent_form();
      }
      if (cell && cell->form_type == dovah::form_type::cell) {
         this->parentCell = _format_cell(cell);
      }
   }
}
void FormUseInfoListModelItem::updateUseInfo(const form_stub& used_form) {
   for (auto& pair : used_form.inbound) {
      if (pair.first != this->otherID)
         continue;
      this->updateUseInfo(pair.second);
      return;
   }
   //
   // If we get here, then we're no longer even used by (used_form).
   //
   this->flags       = 0;
   this->countPlaced = 0;
   this->countUsed   = 0;
}
void FormUseInfoListModelItem::updateUseInfo(const data_t& source) {
   bool is_reference = false;
   if (source.other)
      is_reference = dovah::form_type_is_reference(source.other->form_type) && source.flags & dovah::use_info::entry_flag_to_mask(dovah::use_info::entry_flags::reference::base_form);
   //
   this->flags       = source.flags;
   this->countUsed   = source.refcount;
   this->countPlaced = 0;
   if (is_reference) {
      --this->countUsed;
      ++this->countPlaced;
   }
}

FormUseInfoListModel::FormUseInfoListModel(QObject* parent) : QAbstractTableModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::formCreated ,             this, &FormUseInfoListModel::formCreated);
   QObject::connect(&editor, &DovahKitCore::formModificationImminent, this, &FormUseInfoListModel::formModificationImminent);
   QObject::connect(&editor, &DovahKitCore::formModified,             this, &FormUseInfoListModel::formModified);
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent,     this, &FormUseInfoListModel::formDeletionImminent);
   QObject::connect(&editor, &DovahKitCore::formRenumbered,           this, &FormUseInfoListModel::formRenumbered);
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent,      this, &FormUseInfoListModel::clear);
}
void FormUseInfoListModel::addUser(const use_info_entry& entry, bool queued) {
   //
   // Do not list child forms as "using" their parents, in the UI. The entry 
   // we process here will always be an inbound entry.
   //
   if (entry.flags & dovah::use_info::entry_flag_to_mask(dovah::use_info::entry_flags::base::parent)) {
      if (entry.refcount <= 1)
         return;
   }
   bool is_reference = false;
   if (entry.other)
      is_reference = dovah::form_type_is_reference(entry.other->form_type) && entry.flags & dovah::use_info::entry_flag_to_mask(dovah::use_info::entry_flags::reference::base_form);
   //
   switch (this->mode) {
      case relationship_mode::general_only:
         if (is_reference)
            if (entry.refcount <= 1)
               return;
         break;
      case relationship_mode::base_form_only:
         if (!is_reference)
            return;
         break;
   }
   auto item = new item_type(&entry);
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
void FormUseInfoListModel::removeUser(item_type* item) {
   QModelIndex parent_index;
   auto& list  = this->children;
   auto  index = list.indexOf(item);
   if (index < 0)
      return;
   this->beginRemoveRows(parent_index, index, index);
   list.removeAt(index);
   this->endRemoveRows();
}
void FormUseInfoListModel::updateUser(item_type* item) {
   auto& watch = this->potential_severed_uses;
   auto  index = watch.indexOf(item);
   if (index >= 0)
      watch.removeAt(index);
   //
   item->updateUseInfo(*this->used);
   if (item->isNonUse()) {
      this->removeUser(item);
      return;
   }
   item->updateFromStub();
   //
   auto i     = this->children.indexOf(item);
   auto root  = QModelIndex();
   auto start = this->index(i, 0, root);
   auto end   = this->index(i, this->columnCount(root), root);
   emit dataChanged(start, end);
}

void FormUseInfoListModel::formCreated(const dovah::form_stub* stub) {
   if (!this->used)
      return;
   for (auto& pair : this->used->inbound) {
      if (pair.first != stub->formID)
         continue;
      //
      // A new form was created, and it uses this form (perhaps because it's a duplicate, 
      // or perhaps because it's a brand new form and the form we're currently viewing use 
      // info for is a hardcoded form that the new form uses by default).
      //
      this->addUser(pair.second, false);
      return;
   }
}
void FormUseInfoListModel::formModificationImminent(const dovah::form_stub* stub) {
   if (stub == this->used || !this->used)
      return;
   for (auto* item : this->children) {
      if (item->otherStub == stub) {
         this->potential_severed_uses.push_back(item);
         break;
      }
   }
}
void FormUseInfoListModel::formModified(const dovah::form_stub* stub) {
   if (!this->used)
      return;
   if (stub == this->used)
      return;
   auto& watch = this->potential_severed_uses;
   for (int i = 0; i < watch.size(); ++i) {
      auto* item = watch[i];
      if (item->otherStub == stub) {
         //
         // A known user was altered.
         //
         this->updateUser(item);
         return;
      }
   }
   for (auto& pair : this->used->inbound) {
      if (pair.first != stub->formID)
         continue;
      //
      // A form was altered to become a user.
      //
      this->addUser(pair.second, false);
      return;
   }
}
void FormUseInfoListModel::formDeletionImminent(const dovah::form_stub* stub, bool is_just_flagged) {
   if (!this->used)
      return;
   if (stub == this->used) {
      this->clear();
      return;
   }
   auto& list = this->children;
   auto  size = list.size();
   for (int i = 0; i < size; ++i) {
      auto* item = list[i];
      if (item->otherStub == stub) {
         if (is_just_flagged) {
            this->updateUser(item);
            continue;
         }
         this->removeUser(item);
      }
   }
}
void FormUseInfoListModel::formRenumbered(const dovah::form_stub* stub, dovah::bare_form_id_t oldID, dovah::bare_form_id_t newID) {
   auto& list = this->children;
   auto  size = list.size();
   for (size_t i = 0; i < size; ++i) {
      auto* item = list[i];
      if (item->otherStub == stub) {
         item->updateFromStub();
         auto index = this->index(i, 1, QModelIndex());
         emit dataChanged(index, index);
         return;
      }
   }
}
//
QModelIndex FormUseInfoListModel::index(int row, int column, const QModelIndex& parent) const {
   if (!this->hasIndex(row, column, parent))
      return QModelIndex();
   item_type* childItem = this->children.value(row);
   if (childItem)
      return this->createIndex(row, column, childItem);
   return QModelIndex();
}
QModelIndex FormUseInfoListModel::parent(const QModelIndex& index) const {
   return QModelIndex();
}
int FormUseInfoListModel::rowCount(const QModelIndex& parent) const {
   if (parent.column() > 0)
      return 0;
   return this->children.size();
}
int FormUseInfoListModel::columnCount(const QModelIndex& item) const {
   if (this->mode == relationship_mode::base_form_only) {
      return 3;
   }
   return 4;
}
Qt::ItemFlags FormUseInfoListModel::flags(const QModelIndex& index) const {
   if (!index.isValid())
      return Qt::NoItemFlags;
   return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled;
}
QVariant FormUseInfoListModel::data(const QModelIndex& index, int role) const {
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
            case Qt::UserRole + 1: // filtering
               return QVariant();
         }
         break;
      case 1: // form ID
         switch (role) {
            case Qt::DisplayRole:
               return editor_helpers::form_id_to_string(item->otherID);
            case Qt::UserRole + 0: // sorting
               return item->otherID;
         }
         break;
      case 2: // editor ID or parent cell information
         switch (role) {
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
            case Qt::UserRole + 0: // sorting
            case Qt::UserRole + 1: // filtering
               if (this->mode == relationship_mode::base_form_only) {
                  return item->parentCell;
               }
               if (item->otherType == dovah::form_type::land) {
                  return item->parentCell;
               }
               return item->editorID;
         }
         break;
      case 3: // count
         switch (role) {
            case Qt::DisplayRole:
            case Qt::UserRole + 0: // sorting
               if (this->mode == relationship_mode::general_only)
                  return item->countUsed;
               if (this->mode == relationship_mode::base_form_only)
                  return item->countPlaced;
               return QVariant();
            case Qt::UserRole + 1: // filtering
               return QVariant(); // don't allow filtering
         }
   }
   return QVariant();
}
inline const FormUseInfoListModel::item_type* FormUseInfoListModel::row(int rowIndex) const noexcept {
   return this->children.value(rowIndex);
}
//
QVariant FormUseInfoListModel::headerData(int section, Qt::Orientation orientation, int role) const {
   if (orientation != Qt::Orientation::Horizontal)
      return QVariant();
   switch (role) {
      case Qt::DisplayRole:
         switch (section) {
            case 0: return tr("Type",      "use info report");
            case 1: return tr("Form ID",   "use info report");
            case 2:
               if (this->mode == relationship_mode::base_form_only) {
                  return tr("Parent Cell", "use info report");
               }
               return tr("Editor ID", "use info report");
            case 3:
               if (this->mode == relationship_mode::base_form_only) {
                  return tr("No. Placed", "use info report");
               }
               return tr("Use Count", "use info report");
         }
         break;
   }
   return QVariant();
}

void FormUseInfoListModel::clear() {
   this->beginResetModel();
   this->used = nullptr;
   for (auto* item : this->children)
      delete item;
   this->children.clear();
   this->potential_severed_uses.clear();
   this->endResetModel();
}
void FormUseInfoListModel::build(const dovah::form_stub* used) {
   this->clear();
   if (!used)
      return;
   //
   this->used = used;
   auto& queued = this->queued_additions;
   //
   for (auto& pair : used->inbound)
      this->addUser(pair.second, true);
   if (queued.size() == 0)
      return;
   auto first_inserted = this->children.size();
   auto last_inserted  = first_inserted + queued.size() - 1;
   this->beginInsertRows(QModelIndex(), first_inserted, last_inserted); // we're not passing the count, we're passing the index of the last row. how annoying.
   for(auto* item : queued)
      this->children.push_back(item);
   queued.clear();
   this->endInsertRows();
}
void FormUseInfoListModel::setRelationshipMode(relationship_mode mode) noexcept {
   this->mode = mode;
}
#pragma endregion

FormUseInfoListModelProxy::FormUseInfoListModelProxy(QObject* parent) : QSortFilterProxyModel(parent) {
   this->setFilterCaseSensitivity(Qt::CaseInsensitive);
   this->setFilterRole(Qt::UserRole + 1);
   this->setFilterKeyColumn(-1);
   this->setSortCaseSensitivity(Qt::CaseInsensitive);
   this->setSortRole(Qt::UserRole + 0);
}
bool FormUseInfoListModelProxy::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const {
   if (this->_formType != dovah::form_type::none) {
      auto* model = (model_type*)this->sourceModel();
      auto* item  = model->row(sourceRow);
      if (item && item->otherType != this->_formType) {
         return false;
      }
   }
   return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
}
void FormUseInfoListModelProxy::setFormType(dovah::form_type ft) {
   this->_formType = ft;
   this->invalidateFilter();
}

#pragma region FormUseInfoList
FormUseInfoList::FormUseInfoList(QWidget* parent) : QTableView(parent) {
   {
      auto model = new model_type(this);
      auto proxy = new FormUseInfoListModelProxy(this);
      proxy->setSourceModel(model);
      this->setModel(proxy);
   }
   this->verticalHeader()->setDefaultSectionSize(0);
   this->sortByColumn(0, Qt::AscendingOrder);
   
   this->setSelectionMode(SelectionMode::ExtendedSelection);

   auto header  = this->horizontalHeader();
   auto metrics = QFontMetrics(this->font());
   header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignBaseline);
   header->setMinimumSectionSize(2);
   header->resizeSection(0, metrics.boundingRect("XMMX").width() * 1.5F + 4);
   header->resizeSection(1, 4);
   header->resizeSection(3, metrics.boundingRect("Use Count").width() * 1.5F + 4);
   header->setSectionResizeMode(0, QHeaderView::Interactive);
   header->setSectionResizeMode(1, QHeaderView::Interactive);
   header->setSectionResizeMode(2, QHeaderView::Stretch);
   header->setSectionResizeMode(3, QHeaderView::Interactive);
   //
   QObject::connect(this, &QTableView::doubleClicked, [this](const QModelIndex& index) {
      auto* proxy = (QSortFilterProxyModel*)this->model();
      auto  real  = proxy->mapToSource(index); // the (index) we received is specific to the proxy; we need an index relative to the underlying model
      if (!real.isValid())
         return;
      auto data = (model_item_type*)real.internalPointer();
      if (data) {
         auto* stub = data->otherStub;
         if (stub) {
            if (dovah::form_type_is_reference(stub->form_type)) {
               dovahkit::subsystems::worldedit::core::get_or_create().center_on_refr(*stub);
            } else {
               open_edit_dialog_for_form(*(data->otherStub), this);
            }
         }
      }
   });
   QObject::connect(this->_filterThrottle, &QTimer::timeout, [this]() {
      if (this->_filter)
         this->refilterModelByText(this->_filter->text());
   });
   //
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::formsRenumberedEnMasse, this, &FormUseInfoList::build);
};
FormUseInfoList::relationship_mode FormUseInfoList::relationshipMode() const noexcept {
   auto model = (model_type*)this->unwrappedModel();
   if (!model)
      return relationship_mode::invalid;
   return model->relationshipMode();
}
void FormUseInfoList::setRelationshipMode(relationship_mode m) noexcept {
   auto model = (model_type*)this->unwrappedModel();
   if (!model)
      return;
   model->setRelationshipMode(m);
   model->build(this->target);
}
void FormUseInfoList::refilterModelByText(const QString& text) {
   auto wrapper = (QSortFilterProxyModel*)this->model();
   if (!wrapper)
      return;
   wrapper->setFilterFixedString(text);
}
void FormUseInfoList::textFilterChanged() {
   auto& timer = *this->_filterThrottle;
   if (timer.isActive())
      return;
   timer.start(200);
}
void FormUseInfoList::textFilterFinished() {
   this->_filterThrottle->stop();
   if (this->_filter)
      this->refilterModelByText(this->_filter->text());
}
void FormUseInfoList::setTarget(const dovah::form_stub* stub) {
   this->target = stub;
   auto model = (model_type*)this->unwrappedModel();
   if (!model)
      return;
   model->build(stub);
}
void FormUseInfoList::setTextFilter(QLineEdit* field) {
   this->_filterThrottle->stop();
   if (this->_filter) {
      QObject::disconnect(this->_filter, &QLineEdit::textEdited, this, &FormUseInfoList::textFilterChanged);
      QObject::disconnect(this->_filter, &QLineEdit::editingFinished, this, &FormUseInfoList::textFilterFinished);
   }
   this->_filter = field;
   if (!field)
      return;
   this->refilterModelByText(field->text());
   QObject::connect(field, &QLineEdit::textEdited, this, &FormUseInfoList::textFilterChanged);
   QObject::connect(field, &QLineEdit::editingFinished, this, &FormUseInfoList::textFilterFinished);
}
void FormUseInfoList::setFormTypeFilter(dovah::form_type ft) {
   auto* proxy = (FormUseInfoListModelProxy*)this->model();
   if (!proxy)
      return;
   proxy->setFormType(ft);
}
void FormUseInfoList::build() {
   auto model = (model_type*)this->unwrappedModel();
   if (!model)
      return;
   model->build(this->target);
}
#pragma endregion