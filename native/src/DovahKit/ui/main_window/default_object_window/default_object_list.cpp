#include "default_object_list.h"
#include <QHeaderView>
#include <QLineEdit>
#include "../../../helpers/qt/strings.h"
#include "../../../dovah/forms/DefaultObjectManager.h"
#include "../../../editor/core.h"
#include "../../../editor/get_default_object_info.h"

#pragma region DefaultObjectListModel
DefaultObjectListModelItem::DefaultObjectListModelItem(uint32_t signature, dovah::form_type_t ft) {
   this->signature   = signature;
   this->form_type   = ft;
   this->name        = get_default_object_name(signature);
   this->description = get_default_object_description(signature);
}
QString DefaultObjectListModelItem::valueAsString() const noexcept {
   if (!this->form)
      return QObject::tr("NONE", "default object list - no form");
   uint32_t signature = dovah::form_type_info::lookup(this->form->formType).signature;
   return QString("[%1:%2]%3")
      .arg(cobb::qt::four_cc_to_string(signature))
      .arg(QString("%1").arg(this->form->formID, 8, 16, QChar('0')).toUpper())
      .arg(this->form->get_editor_id());
}

DefaultObjectListModel::DefaultObjectListModel(QObject* parent) : QAbstractTableModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent,       this, &DefaultObjectListModel::build);
   QObject::connect(&editor, &DovahKitCore::dataAcquireComplete,       this, &DefaultObjectListModel::build);
   QObject::connect(&editor, &DovahKitCore::formModified,              this, &DefaultObjectListModel::formModified);
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent,      this, &DefaultObjectListModel::formDeletionImminent);
   QObject::connect(&editor, &DovahKitCore::formRenumbered,            this, &DefaultObjectListModel::formRenumbered);
   QObject::connect(&editor, &DovahKitCore::formsRenumberedEnMasse,    this, &DefaultObjectListModel::build);
   QObject::connect(&editor, &DovahKitCore::defaultObjectEntryChanged, this, &DefaultObjectListModel::defaultObjectEntryChanged);
}
//
void DefaultObjectListModel::defaultObjectEntryChanged(uint32_t signature) {
   int size = this->children.size();
   for (int i = 0; i < size; ++i) {
      auto* item = this->children[i];
      if (item->signature == signature) {
         auto& editor = DovahKitCore::get();
         form_ptr_t loaded;
         if (auto* stub = editor.get_singleton_form(dovah::form_type::default_object_manager)) {
            loaded = stub->load().ptr_cast<form_t>();
            if (!loaded)
               return;
         }
         item->form      = loaded->get_entry(signature);
         item->is_edited = loaded->entry_is_edited(item->signature);
         //
         auto root  = QModelIndex();
         auto start = this->index(i, 0, root);
         auto end   = this->index(i, this->columnCount(root), root);
         emit dataChanged(start, end);
         return;
      }
   }
}
void DefaultObjectListModel::formModified(const dovah::form_stub* stub) {
   int size = this->children.size();
   for (int i = 0; i < size; ++i) {
      auto* item = this->children[i];
      if (item->form != stub)
         continue;
      auto root  = QModelIndex();
      auto start = this->index(i, 0, root);
      auto end   = this->index(i, this->columnCount(root), root);
      emit dataChanged(start, end);
   }
}
void DefaultObjectListModel::formDeletionImminent(const dovah::form_stub* stub, bool is_just_flagged) {
   auto& editor = DovahKitCore::get();
   form_ptr_t loaded;
   if (auto* stub = editor.get_singleton_form(dovah::form_type::default_object_manager)) {
      loaded = stub->load().ptr_cast<form_t>();
   }
   //
   int size = this->children.size();
   for (int i = 0; i < size; ++i) {
      auto* item = this->children[i];
      if (item->form != stub)
         continue;
      item->form = nullptr;
      if (loaded)
         item->is_edited = loaded->entry_is_edited(item->signature);
      auto root  = QModelIndex();
      auto start = this->index(i, 0, root);
      auto end   = this->index(i, this->columnCount(root), root);
      emit dataChanged(start, end);
   }
}
void DefaultObjectListModel::formRenumbered(const dovah::form_stub* stub, dovah::bare_form_id_t oldID, dovah::bare_form_id_t newID) {
   int size = this->children.size();
   for (int i = 0; i < size; ++i) {
      auto* item = this->children[i];
      if (item->form != stub)
         continue;
      auto root  = QModelIndex();
      auto start = this->index(i, 0, root);
      auto end   = this->index(i, this->columnCount(root), root);
      emit dataChanged(start, end);
   }
}
//
QModelIndex DefaultObjectListModel::index(int row, int column, const QModelIndex& parent) const {
   if (!this->hasIndex(row, column, parent))
      return QModelIndex();
   item_type* childItem = this->children.value(row);
   if (childItem)
      return this->createIndex(row, column, childItem);
   return QModelIndex();
}
QModelIndex DefaultObjectListModel::parent(const QModelIndex& index) const {
   return QModelIndex();
}
int DefaultObjectListModel::rowCount(const QModelIndex& parent) const {
   if (parent.column() > 0)
      return 0;
   return this->children.size();
}
int DefaultObjectListModel::columnCount(const QModelIndex& item) const {
   return 3;
}
Qt::ItemFlags DefaultObjectListModel::flags(const QModelIndex& index) const {
   if (!index.isValid())
      return Qt::NoItemFlags;
   return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled;
}
QVariant DefaultObjectListModel::data(const QModelIndex& index, int role) const {
   if (!index.isValid())
      return QVariant();
   auto item   = (item_type*)index.internalPointer();
   auto column = index.column();
   switch (column) {
      case ColumnSignature:
         switch (role) {
            case Qt::DisplayRole:
               return QObject::tr("%1%2").arg(cobb::qt::four_cc_to_string(item->signature)).arg(item->is_edited ? QObject::tr(" *", "default object window - active file setting marker") : "");
            case SortRole: // sorting
            case FilterRole: // filtering
               return cobb::qt::four_cc_to_string(item->signature);
            case Qt::TextAlignmentRole:
               return Qt::AlignCenter;
         }
         break;
      case ColumnName:
         switch (role) {
            case Qt::DisplayRole:
               return QObject::tr("%1%2").arg(item->name).arg(item->is_edited ? QObject::tr(" *", "default object window - active file setting marker") : "");
            case SortRole: // sorting
            case FilterRole: // filtering
               return item->name;
         }
         break;
      case ColumnValue:
         switch (role) {
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
            case SortRole: // sorting
            case FilterRole: // filtering
               return item->valueAsString();
            case Qt::TextAlignmentRole:
               return Qt::AlignLeft;
         }
         break;
   }
   return QVariant();
}
inline const DefaultObjectListModel::item_type* DefaultObjectListModel::row(int rowIndex) const noexcept {
   return this->children.value(rowIndex);
}
//
QVariant DefaultObjectListModel::headerData(int section, Qt::Orientation orientation, int role) const {
   if (orientation != Qt::Orientation::Horizontal)
      return QVariant();
   switch (role) {
      case Qt::DisplayRole:
         switch (section) {
            case ColumnSignature: return tr("ID",    "default object list");
            case ColumnName:      return tr("Name",  "default object list");
            case ColumnValue:     return tr("Value", "default object list");
         }
         break;
   }
   return QVariant();
}

void DefaultObjectListModel::clear() {
   this->beginResetModel();
   for (auto* item : this->children)
      delete item;
   this->children.clear();
   for (auto* item : this->queued_additions)
      delete item;
   this->queued_additions.clear();
   this->endResetModel();
}
void DefaultObjectListModel::build() {
   this->clear();
   //
   form_ptr_t loaded;
   //
   auto& editor = DovahKitCore::get();
   auto& queued = this->queued_additions;
   auto* stub   = editor.get_singleton_form(dovah::form_type::default_object_manager);
   if (stub)
      loaded = stub->load().ptr_cast<form_t>();
   for (auto& definition : dovah::default_objects) {
      auto* item = new item_type(definition.signature, definition.type);
      if (loaded) {
         item->form      = loaded->get_entry(definition.signature);
         item->is_edited = loaded->entry_is_edited(definition.signature);
      }
      queued.push_back(item);
   }
   if (loaded) {
      for (auto& pair : loaded->entries) {
         auto  signature = pair.first;
         auto& entry     = pair.second;
         if (dovah::get_default_object_definition(signature))
            continue;
         auto* item = new item_type(signature, dovah::form_type::none);
         item->form      = entry.form.get_form_stub();
         item->is_edited = entry.is_active_file;
         queued.push_back(item);
      }
   }
   if (queued.size() == 0)
      return;
   auto first_inserted = this->children.size();
   auto last_inserted  = first_inserted + queued.size() - 1;
   this->beginInsertRows(QModelIndex(), first_inserted, last_inserted); // we're not passing the count; we're passing the index of the last row.
   for(auto* item : queued)
      this->children.push_back(item);
   queued.clear();
   this->endInsertRows();
}
#pragma endregion

DefaultObjectListModelProxy::DefaultObjectListModelProxy(QObject* parent) : QSortFilterProxyModel(parent) {
   this->setFilterCaseSensitivity(Qt::CaseInsensitive);
   this->setFilterRole(DefaultObjectListModel::FilterRole);
   this->setFilterKeyColumn(-1);
   this->setSortCaseSensitivity(Qt::CaseInsensitive);
   this->setSortRole(DefaultObjectListModel::SortRole);
}

#pragma region DefaultObjectList
DefaultObjectList::DefaultObjectList(QWidget* parent) : QTableView(parent) {
   {
      auto model = new model_type(this);
      auto proxy = new DefaultObjectListModelProxy(this);
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
   header->resizeSection(DefaultObjectListModel::ColumnSignature, metrics.boundingRect("XMMX *").width() * 1.5F + 4);
   header->setSectionResizeMode(DefaultObjectListModel::ColumnName,  QHeaderView::Stretch);
   header->setSectionResizeMode(DefaultObjectListModel::ColumnValue, QHeaderView::Stretch);
   //
   QObject::connect(this->_filterThrottle, &QTimer::timeout, [this]() {
      if (this->_filter)
         this->refilterModelByText(this->_filter->text());
   });
   //
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::formsRenumberedEnMasse, this, &DefaultObjectList::build);
};
void DefaultObjectList::refilterModelByText(const QString& text) {
   auto wrapper = (QSortFilterProxyModel*)this->model();
   if (!wrapper)
      return;
   wrapper->setFilterFixedString(text);
}
void DefaultObjectList::textFilterChanged() {
   auto& timer = *this->_filterThrottle;
   if (timer.isActive())
      return;
   timer.start(200);
}
void DefaultObjectList::textFilterFinished() {
   this->_filterThrottle->stop();
   if (this->_filter)
      this->refilterModelByText(this->_filter->text());
}
void DefaultObjectList::setTextFilter(QLineEdit* field) {
   this->_filterThrottle->stop();
   if (this->_filter) {
      QObject::disconnect(this->_filter, &QLineEdit::textEdited, this, &DefaultObjectList::textFilterChanged);
      QObject::disconnect(this->_filter, &QLineEdit::editingFinished, this, &DefaultObjectList::textFilterFinished);
   }
   this->_filter = field;
   if (!field)
      return;
   this->refilterModelByText(field->text());
   QObject::connect(field, &QLineEdit::textEdited, this, &DefaultObjectList::textFilterChanged);
   QObject::connect(field, &QLineEdit::editingFinished, this, &DefaultObjectList::textFilterFinished);
}
void DefaultObjectList::build() {
   auto model = (model_type*)this->unwrappedModel();
   if (!model)
      return;
   model->build();
}
#pragma endregion