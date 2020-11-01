#include "game_setting_list.h"
#include <QHeaderView>
#include <QLineEdit>
#include "../../../helpers/qt/strings.h"
#include "../../../editor/core.h"
#include "../../../editor/get_game_setting_description.h"

#pragma region GameSettingListModel
GameSettingListModelItem::GameSettingListModelItem(const dovah::loaded_game_setting& source) {
   this->updateFrom(source);
}
void GameSettingListModelItem::updateFrom(const dovah::loaded_game_setting& source) {
   if (auto* def = source.definition) {
      this->name        = QString::fromLatin1(def->name);
      this->description = get_game_setting_description(def->name);
   } else {
      this->name = QString::fromStdString(source.name);
   }
   this->type   = source.get_type();
   this->formID = source.formID;
   switch (this->type) {
      case dovah::game_setting_type::boolean:
         this->value.boolean = source.value.b;
         break;
      case dovah::game_setting_type::float32:
         this->value.float32 = source.value.f;
         break;
      case dovah::game_setting_type::integer:
         this->value.number = source.value.i;
         break;
      case dovah::game_setting_type::string:
         this->value.string = DovahKitCore::get().convert_localized_string(source.value.s);
         break;
   }
   //
   // TODO: get last file to define the setting
   //
}
QString GameSettingListModelItem::valueAsString() const noexcept {
   switch (this->type) {
      case dovah::game_setting_type::boolean:
         if (this->value.boolean)
            return QObject::tr("true", "game setting value (boolean)");
         return QObject::tr("false", "game setting value (boolean)");
      case dovah::game_setting_type::float32:
         return QString("%1").arg(this->value.float32);
      case dovah::game_setting_type::integer:
         return QString("%1").arg(this->value.number);
      case dovah::game_setting_type::string:
         return this->value.string;
   }
   return QString();
}

GameSettingListModel::GameSettingListModel(QObject* parent) : QAbstractTableModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent,     this, &GameSettingListModel::formDeletionImminent);
   QObject::connect(&editor, &DovahKitCore::formRenumbered,           this, &GameSettingListModel::formRenumbered);
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent,      this, &GameSettingListModel::clear);
}
void GameSettingListModel::addUser(const use_info_entry& entry, bool queued) {
   using _ue_flag = dovah::use_info_entry::flag;
   //
   // Do not list child forms as "using" their parents, in the UI:
   //
   if (entry.flags & (_ue_flag::i_am_parent_of)) {
      if (entry.refcount <= 1)
         return;
   }
   //
   switch (this->mode) {
      case relationship_mode::general_only:
         if (entry.flags & _ue_flag::object_reference)
            if (entry.refcount <= 1)
               return;
         break;
      case relationship_mode::base_form_only:
         if (!(entry.flags & _ue_flag::object_reference))
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
void GameSettingListModel::removeUser(item_type* item) {
   QModelIndex parent_index;
   auto& list  = this->children;
   auto  index = list.indexOf(item);
   if (index < 0)
      return;
   this->beginRemoveRows(parent_index, index, index);
   list.removeAt(index);
   this->endRemoveRows();
}
void GameSettingListModel::updateUser(item_type* item) {
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

void GameSettingListModel::formDeletionImminent(const dovah::form_stub* stub, bool is_just_flagged) {
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
void GameSettingListModel::formRenumbered(const dovah::form_stub* stub, dovah::bare_form_id_t oldID, dovah::bare_form_id_t newID) {
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
QModelIndex GameSettingListModel::index(int row, int column, const QModelIndex& parent) const {
   if (!this->hasIndex(row, column, parent))
      return QModelIndex();
   item_type* childItem = this->children.value(row);
   if (childItem)
      return this->createIndex(row, column, childItem);
   return QModelIndex();
}
QModelIndex GameSettingListModel::parent(const QModelIndex& index) const {
   return QModelIndex();
}
int GameSettingListModel::rowCount(const QModelIndex& parent) const {
   if (parent.column() > 0)
      return 0;
   return this->children.size();
}
int GameSettingListModel::columnCount(const QModelIndex& item) const {
   return 4;
}
Qt::ItemFlags GameSettingListModel::flags(const QModelIndex& index) const {
   if (!index.isValid())
      return Qt::NoItemFlags;
   return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled;
}
QVariant GameSettingListModel::data(const QModelIndex& index, int role) const {
   if (!index.isValid())
      return QVariant();
   auto item   = (item_type*)index.internalPointer();
   auto column = index.column();
   switch (column) {
      case ColumnName:
         switch (role) {
            case Qt::DisplayRole:
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
         }
         break;
      case ColumnFormID:
         switch (role) {
            case Qt::DisplayRole:
               return QString("%1").arg(item->formID, 8, 16, QChar('0')).toUpper();
            case SortRole: // sorting
               return item->formID;
         }
         break;
      case ColumnFile:
         switch (role) {
            case Qt::DisplayRole:
            case SortRole: // sorting
            case FilterRole: // filtering
               return item->last_file;
         }
   }
   return QVariant();
}
inline const GameSettingListModel::item_type* GameSettingListModel::row(int rowIndex) const noexcept {
   return this->children.value(rowIndex);
}
//
QVariant GameSettingListModel::headerData(int section, Qt::Orientation orientation, int role) const {
   if (orientation != Qt::Orientation::Horizontal)
      return QVariant();
   switch (role) {
      case Qt::DisplayRole:
         switch (section) {
            case ColumnName:   return tr("Name",        "game setting list");
            case ColumnValue:  return tr("Value",       "game setting list");
            case ColumnFormID: return tr("Form ID",     "game setting list");
            case ColumnFile:   return tr("Source File", "game setting list");
         }
         break;
   }
   return QVariant();
}

void GameSettingListModel::clear() {
   this->beginResetModel();
   for (auto* item : this->children)
      delete item;
   this->children.clear();
   this->endResetModel();
}
void GameSettingListModel::build() {
   this->clear();
   //
   auto& queued = this->queued_additions;
   //
   using _ue_flag = dovah::use_info_entry::flag;
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
#pragma endregion

GameSettingListModelProxy::GameSettingListModelProxy(QObject* parent) : QSortFilterProxyModel(parent) {
   this->setFilterCaseSensitivity(Qt::CaseInsensitive);
   this->setFilterRole(GameSettingListModel::FilterRole);
   this->setFilterKeyColumn(-1);
   this->setSortCaseSensitivity(Qt::CaseInsensitive);
   this->setSortRole(GameSettingListModel::SortRole);
}

#pragma region GameSettingList
GameSettingList::GameSettingList(QWidget* parent) : QTableView(parent) {
   {
      auto model = new model_type(this);
      auto proxy = new GameSettingListModelProxy(this);
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
      if (data && data->otherStub)
         open_edit_dialog_for_form(data->otherStub, this);
   });
   QObject::connect(this->_filterThrottle, &QTimer::timeout, [this]() {
      if (this->_filter)
         this->refilterModelByText(this->_filter->text());
   });
   //
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::formsRenumberedEnMasse, this, &GameSettingList::build);
};
void GameSettingList::refilterModelByText(const QString& text) {
   auto wrapper = (QSortFilterProxyModel*)this->model();
   if (!wrapper)
      return;
   wrapper->setFilterFixedString(text);
}
void GameSettingList::textFilterChanged() {
   auto& timer = *this->_filterThrottle;
   if (timer.isActive())
      return;
   timer.start(200);
}
void GameSettingList::textFilterFinished() {
   this->_filterThrottle->stop();
   if (this->_filter)
      this->refilterModelByText(this->_filter->text());
}
void GameSettingList::setTextFilter(QLineEdit* field) {
   this->_filterThrottle->stop();
   if (this->_filter) {
      QObject::disconnect(this->_filter, &QLineEdit::textEdited, this, &GameSettingList::textFilterChanged);
      QObject::disconnect(this->_filter, &QLineEdit::editingFinished, this, &GameSettingList::textFilterFinished);
   }
   this->_filter = field;
   if (!field)
      return;
   this->refilterModelByText(field->text());
   QObject::connect(field, &QLineEdit::textEdited, this, &GameSettingList::textFilterChanged);
   QObject::connect(field, &QLineEdit::editingFinished, this, &GameSettingList::textFilterFinished);
}
void GameSettingList::build() {
   auto model = (model_type*)this->unwrappedModel();
   if (!model)
      return;
   model->build();
}
#pragma endregion