#include "form_table.h"
#include <QHeaderView>
#include <QLineEdit>
#include <QMimeData>
#include "../../../editor/core.h"
#include "../../../editor/helpers/form_identifiers_to_string.h"
#include "../../../editor/helpers/form_stub_drag_drop.h"
#include "../../../dovah/form_stub.h"
#include "../../../dovah/files/common.h"

#include "dovah/data/story_manager.h"
#include "editor/helpers/story_event_name.h"
#include "editor/subsystems/story_manager/core.h"

//
// KNOWN DEFECTS:
//
//  - QSortFilterProxyModel has internal mappings that it needs to build; this causes a 
//    lag spike when the user changes the Object Window's filter for the first time after 
//    files are loaded
//

namespace {
   // Qt's implementation of QSortFilterProxyModel filters always goes through QRegExp and 
   // friends, even when you just filter by a fixed string. QRegExp objects can wrap a fixed 
   // string rather than a real regex, but even in that case, it still incurs the overhead 
   // of regex matching, including locking to access a global cache of regex results.
   constexpr const bool replace_qt_filter_string_handling = false;

   // Ignores QSortFilterProxyModel's parameters for filtering, and uses hardcoded ones.
   constexpr const bool replace_qt_filter_string_params = false;

   //
   // The above options don't really help much. I think the only solution to the lag we're 
   // seeing in Debug would be to build a custom sort/filter proxy model with cheaper mappings. 
   // QSortFilterProxyModel is designed to support recursive filtering if you enable it (it's 
   // disabled by default, and we obviously don't use it here), and as a result, its design 
   // incurs overhead for that:
   // 
   //  - The proxy stores its mappings as a vector of source-to-proxy row indices, a vector of 
   //    the reverse, and another pair of vectors for column indices. However, the proxy is 
   //    capable of storing multiple mappings keyed to different parent QModelIndexes, and it 
   //    has to find the appropriate index before it can update any mapping.
   // 
   //  - Added branching at every filter step, for features we don't use, e.g. checking whether 
   //    recursive filtering is enabled for each individual row.
   // 
   //  - Filter code per-column, even though we don't filter the columns.
   // 
   //  - We can only filter rows based on the contents of one column, or every column. What we 
   //    really want is to filter just specific columns (currently indices 0 and 1).
   // 
   // Additionally, everything is done via virtual member functions, rather than via functions 
   // that can be inlined. This includes the per-row checks. Hard to measure the overhead that 
   // that adds without something to compare it to, though.
   // 
   // Ideally we'd make a model that stores only mappings for the root node's top-level children, 
   // with compile-time options rather than run-time ones.
   //
}

FormTableModelItem::FormTableModelItem(dovah::form_stub* stub) {
   this->stub = stub;
   this->update();
}
void FormTableModelItem::update() {
   auto stub = this->stub;
   this->editorID = QString::fromUtf8(stub->get_editor_id());
   if (stub->form_type == dovah::form_type::story_event_node) {
      //
      // It's common for these forms to have no editor ID, and in fact the CK doesn't 
      // even let you give them one. Instead, the Object Window should identify them 
      // by their event typename.
      //
      auto& sm     = dovahkit::subsystems::story_manager::core::get_or_create();
      auto  et_opt = sm.event_type_for(*stub);
      if (et_opt.has_value()) {
         auto et   = et_opt.value();
         auto name = editor_helpers::story_event_name(et);
         if (!name.isEmpty()) {
            this->editorID = name;
         }
      }
   }
   this->formID    = stub->formID;
   this->userCount = stub->inbound.size();
   
   this->is_active = false;
   if (!stub->test_record_flags(dovah::tes_file_record_header::flag::partial)) {
      if (stub->is_edited())
         this->is_active = true;
      else {
         this->is_active = stub->get_owning_load_order().is_defined_or_overridden_in_active_file(*stub);
      }
   }

   this->is_injected = stub->is_injected();
   this->is_none     = stub->is_none_stub();
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
FormTableModel::FormTableModel(QObject* parent) : QAbstractTableModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent,      this, &FormTableModel::clear);
   QObject::connect(&editor, &DovahKitCore::formCreated,              this, &FormTableModel::formCreated);
   QObject::connect(&editor, &DovahKitCore::formModificationImminent, this, &FormTableModel::formModificationImminent);
   QObject::connect(&editor, &DovahKitCore::formModified,             this, &FormTableModel::formModified);
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent,     this, &FormTableModel::formDeletionImminent);
   QObject::connect(&editor, &DovahKitCore::formRenumbered,           this, &FormTableModel::formRenumbered);
}

void FormTableModel::formCreated(dovah::form_stub* stub) {
   if (!this->form_types.contains(stub->form_type))
      return;
   this->insertItem(stub, false);
}
void FormTableModel::formModificationImminent(const dovah::form_stub* user) {
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
void FormTableModel::formModified(const dovah::form_stub* stub) {
   QVector<dovah::form_stub*> used;
   for (auto& pair : stub->outbound) {
      auto* other = pair.second.other;
      if (other)
         used.push_back(other);
   }
   //
   auto  parent_index = QModelIndex();
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
      } else if (used.contains(item->stub)) {
         //
         // Update any other forms that need their use counts used because (stub) was 
         // changed to use them.
         //
         if (item->updateUserCount()) {
            auto root  = QModelIndex();
            auto start = this->index(i, 0, root);
            auto end   = this->index(i, this->columnCount(root), root);
            emit dataChanged(start, end);
         }
      }
   }
   //
   this->doUseInfoUpdate();
}
void FormTableModel::formDeletionImminent(const dovah::form_stub* stub, bool is_just_flagged) {
   auto& list = this->children;
   auto  size = list.size();
   for (size_t i = 0; i < size; ++i) {
      auto* item = list[i];
      if (item->stub == stub) {
         if (is_just_flagged) {
            auto index = this->index(i, 0, QModelIndex());
            emit dataChanged(index, index); // force a redraw, which will show the "deleted" flag
            break;
         }
         this->beginRemoveRows(QModelIndex(), i, i);
         list.remove(i);
         this->endRemoveRows();
         break;
      }
   }
}
void FormTableModel::formRenumbered(const dovah::form_stub* stub, dovah::bare_form_id_t oldID, dovah::bare_form_id_t newID) {
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
         return;
      }
   }
}

QModelIndex FormTableModel::index(dovah::form_stub* stub) const {
   int size = this->children.size();
   for (int i = 0; i < size; ++i)
      if (this->children[i]->stub == stub)
         return this->index(i, 0, QModelIndex());
   return QModelIndex();
}
QModelIndex FormTableModel::index(int row, int column, const QModelIndex& parent) const {
   if (!this->hasIndex(row, column, parent))
      return QModelIndex();
   item_type* childItem = this->children.value(row);
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
   return this->children.size();
}
int FormTableModel::columnCount(const QModelIndex& item) const {
   return 3;
}
Qt::ItemFlags FormTableModel::flags(const QModelIndex& index) const {
   if (!index.isValid())
      return Qt::NoItemFlags;
   return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsDragEnabled;
}
QVariant FormTableModel::data(const QModelIndex& index, int role) const {
   if (!index.isValid())
      return QVariant();
   auto item = (item_type*)index.internalPointer();
   if (!item->stub)
      return QVariant();
   auto column  = index.column();
   bool edited  = item->is_active;
   bool deleted = item->stub->is_deleted();
   bool none    = item->is_none;
   switch (role) {
      case Qt::DisplayRole:
         switch (column) {
            case 0:
               if (none) {
                  return tr("<non-existent form>%1", "object window listing for none stubs")
                     .arg((edited || deleted) ? tr(" * ", "edited form editor ID marker") : "");
               }
               return tr("%1%2")
                  .arg(item->editorID)
                  .arg((edited || deleted) ? tr(" * ", "edited form editor ID marker") : "");
            case 1:
               return editor_helpers::form_id_to_string(item->formID) + ((edited || deleted) ? tr(" * ", "edited form ID marker") : "") + (deleted ? tr("D", "deleted form ID marker") : "");
            case 2:
               return item->userCount;
         }
         break;
      case Qt::DecorationRole:
         if (column == 0) {
            //
            // TODO: icons per form type
            //
         }
         break;
      case Qt::FontRole:
         if (column == 0 && none) { // show none-stubs in italics
            QFont font;
            font.setItalic(true);
            return font;
         }
         break;
      case Qt::ForegroundRole:
         if (column == 1 && item->is_injected) // show injected forms' IDs in color
            return QColor::fromRgb(0x309000);
         break;
      case RawDataRole:
         switch (column) {
            case 0: return item->editorID;
            case 1: return item->formID;
            case 2: return item->userCount;
         }
         break;
      case FilterableTextRole: // used for filtering
         switch (column) {
            case 0:
               if (none)
                  return QVariant();
               return item->editorID;
            case 1: return editor_helpers::form_id_to_string(item->formID);
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
QMimeData* FormTableModel::mimeData(const QModelIndexList& indexes) const {
   //
   // We want the user to be able to drag selected forms out of the Object Window and into 
   // things like a FormList's form list. To do that, we have to send the data along with 
   // a MIME type. We've chosen "application/dovah-kit.form-id-array" as our MIME type; 
   // the underlying data is just form IDs binary-encoded, separated with null bytes.
   //

   QMimeData* out = new QMimeData;

   std::vector<dovah::form_stub*> stubs;

   auto& list = this->children;
   auto  size = list.size();
   for (const QModelIndex& index : indexes) {
      if (index.column() != 0) // row selection + table with multiple columns = multiple indices that represent the same row, one for each column. skip the extras
         continue;
      if (index.isValid()) {
         auto i = index.row();
         if (i < 0 || i >= size)
            continue;
         auto* item = this->children[index.row()];
         if (!item->stub)
            continue;

         stubs.push_back(item->stub);
      }
   }

   editor_helpers::add_form_stubs_to_mime_data(*out, stubs);

   return out;
}
QStringList FormTableModel::mimeTypes() const {
   return QStringList({
      QString(editor_helpers::form_stub_array_mime_type),
      QString(editor_helpers::single_form_stub_mime_type)
   });
}

void FormTableModel::doUseInfoUpdate() {
   auto  parent_index = QModelIndex();
   auto& list = this->children;
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
void FormTableModel::insertItem(dovah::form_stub* stub, bool queued) {
   auto* item = new item_type(stub);
   if (queued) {
      this->pending_additions.push_back(item);
   } else {
      auto& list  = this->children;
      auto  first = list.size();
      this->beginInsertRows(QModelIndex(), first, first);
      list.push_back(item);
      this->endInsertRows();
   }
}

void FormTableModel::clear() {
   this->beginResetModel();
   for (auto* item : this->children)
      delete item;
   this->children.clear();
   this->pending_additions.clear();
   this->forms_pending_use_info_update.clear();
   this->endResetModel();
}
void FormTableModel::rebuild() {
   this->clear();
   //
   if (this->form_types.empty())
      return;
   auto& editor = DovahKitCore::get();
   if (!editor.has_data())
      return;
   //
   {
      bool any = false;
      for(auto ft : this->form_types)
         if (editor.count_forms_of_type(ft)) {
            any = true;
            break;
         }
      if (!any)
         return;
   }
   for (auto ft : this->form_types) {
      if (ft == dovah::form_type::none)
         editor.for_each_form_of_type(ft, [this](dovah::form_stub* stub) {
            if (stub->is_none_stub())
               this->insertItem(stub, true);
            return false;
         });
      else
         editor.for_each_form_of_type(ft, [this](dovah::form_stub* stub) {
            this->insertItem(stub, true);
            return false;
         });
   }
   //
   auto count = this->pending_additions.size();
   if (!count)
      return;
   auto first = this->children.size();
   auto last  = first + count - 1;
   //
   this->beginInsertRows(QModelIndex(), first, last);
   for (auto* item : this->pending_additions)
      this->children.push_back(item);
   this->pending_additions.clear();
   this->endInsertRows();
}
void FormTableModel::setBaseFormTypes(const form_type_set& types) {
   this->form_types = types;
}

const FormTableModel::item_type* FormTableModel::dataAtRow(int row) const noexcept {
   if (row < 0)
      return nullptr;
   if (row >= this->children.size())
      return nullptr;
   return this->children[row];
}
#pragma endregion

#pragma region FormTableModelProxy
void FormTableModelProxy::setSourceModel(QAbstractItemModel* source_model) {
   if (source_model && !qobject_cast<FormTableModel*>(source_model))
      source_model = nullptr;
   QSortFilterProxyModel::setSourceModel(source_model);
}

void FormTableModelProxy::setFilterInfo(const ui::object_window::filter_info& fi) {
   #if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
      this->beginFilterChange();
   #endif
   auto& prior = this->form_filter_info;
   if (prior == fi)
      return;
   prior = fi;
   #if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
      this->endFilterChange(QSortFilterProxyModel::Direction::Rows);
   #else
      this->invalidateFilter();
   #endif
}

bool FormTableModelProxy::filterAcceptsStub(const dovah::form_stub* stub) const noexcept {
   return this->form_filter_info.form_matches_filters(*stub);
}
bool FormTableModelProxy::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const {
   auto* model = (FormTableModel*)this->sourceModel();
   if (!this->form_filter_info.empty()) {
      if (auto* item = model->dataAtRow(source_row)) {
         if (!this->filterAcceptsStub(item->stub))
            return false;
      }
   }
   if constexpr (replace_qt_filter_string_handling) {
      QString filter_string;
      {
         //
         // Qt stores a fixed-string pattern as QRegExp, not QRegularExpression, and 
         // the former is accessible only via an undocumented getter.
         // 
         // I... think the API they designed for this may be a bit poorly thought out.
         //
         auto regex = this->filterRegExp();
         if (regex.patternSyntax() == QRegExp::PatternSyntax::FixedString) {
            filter_string = regex.pattern();
         }
      }
      if (!filter_string.isEmpty()) {
         if constexpr (replace_qt_filter_string_params) {
            for (size_t i = 0; i < 2; ++i) {
               auto qmi  = model->index(source_row, i, source_parent);
               auto data = model->data(qmi, FormTableModel::FilterableTextRole).toString();
               if (data.contains(filter_string, Qt::CaseInsensitive))
                  return true;
            }
            return false;
         } else {
            const auto case_sensitivity = this->filterCaseSensitivity();
            const auto filter_role = this->filterRole();

            const auto col = this->filterKeyColumn();
            const auto col_count = model->columnCount(source_parent);
            if (col == -1) {
               for (int i = 0; i < col_count; ++i) {
                  auto qmi = model->index(source_row, i, source_parent);
                  auto subject = model->data(qmi, filter_role).toString();
                  if (subject.contains(filter_string, case_sensitivity))
                     return true;
               }
               return false;
            } else {
               auto qmi = model->index(source_row, col, source_parent);
               auto subject = model->data(qmi, filter_role).toString();
               return subject.contains(filter_string, case_sensitivity);
            }
         }
      }
      return true;
   } else {
      return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
   }
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
   this->setDragDropMode(QAbstractItemView::DragOnly);
   this->setDragEnabled(true);
   this->setDragDropOverwriteMode(false);
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
   QObject::connect(&editor, &DovahKitCore::dataAcquireComplete,    this, &FormTable::rebuildModel);
   QObject::connect(&editor, &DovahKitCore::formsRenumberedEnMasse, this, &FormTable::rebuildModel);

   QObject::connect(this->_filterThrottle, &QTimer::timeout, [this]() {
      if (this->_filter)
         this->refilterModel(this->_filter->text());
   });
};
void FormTable::recheckFormTypes() {
   if (!this->_source)
      return;
   auto* model = this->proxyModel();
   assert(model);
   model->setFilterInfo(this->_source->filterInfo());
}
void FormTable::rebuildModel() {
   auto* proxy = this->proxyModel();
   auto* model = this->unwrappedModel();
   assert(model);
   model->rebuild();
}
void FormTable::refilterModel(const QString& text) {
   auto* proxy = this->proxyModel();
   assert(proxy);
   proxy->setFilterFixedString(text);
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
   auto* model = this->unwrappedModel();
   assert(model);
   model->clear();
}
void FormTable::select(dovah::form_stub* stub) {
   auto* proxy = (proxy_type*)this->model();
   assert(proxy);
   auto* model = (model_type*)proxy->sourceModel();
   assert(model);
   auto  index  = model->index(stub);
   auto  mapped = proxy->mapFromSource(index);
   auto* select_model = this->selectionModel();
   if (!select_model)
      return;
   select_model->select(mapped, QItemSelectionModel::ClearAndSelect);
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
void FormTable::setSource(ObjectWindowTree* tree) {
   if (this->_source)
      QObject::disconnect(this->_source->selectionModel(), &QItemSelectionModel::selectionChanged , this, &FormTable::recheckFormTypes);
   this->_source = tree;
   if (!tree)
      return;
   this->unwrappedModel()->setBaseFormTypes(tree->allPrimaryFormTypes());
   QObject::connect(tree->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FormTable::recheckFormTypes);
   this->recheckFormTypes();
}
#pragma endregion