#include "game_setting_list.h"
#include <QHeaderView>
#include <QLineEdit>
#include "../../../helpers/qt/strings.h"
#include "../../../dovah/files/tes_file_reading/file.h"
#include "../../../editor/core.h"
#include "../../../editor/get_game_setting_description.h"

#pragma region GameSettingListModel
GameSettingListModelItem::GameSettingListModelItem(const dovah::loaded_game_setting& source) {
   this->updateFrom(source);
}
GameSettingListModelItem::GameSettingListModelItem(const dovah::game_setting_definition& definition) {
   this->name        = QString::fromLatin1(definition.name);
   this->description = get_game_setting_description(definition.name);
   this->type        = definition.type;
   switch (this->type) {
      case dovah::game_setting_type::boolean:
         this->value.boolean = definition.default_value.b;
         break;
      case dovah::game_setting_type::float32:
         this->value.float32 = definition.default_value.f;
         break;
      case dovah::game_setting_type::integer:
         this->value.number = definition.default_value.i;
         break;
      case dovah::game_setting_type::string:
         this->value.string = definition.default_value.s.c_str();
         break;
   }
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
   if (source.source_file) {
      this->last_file = QString::fromStdString(source.source_file->get_filename());
      if (this->last_file.isEmpty()) {
         this->last_file = QObject::tr("<untitled>", "unsaved/implicit active file shown in game setting window");
      }
   }
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
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent,     this, &GameSettingListModel::clear);
   QObject::connect(&editor, &DovahKitCore::gameSettingValueChanged, this, &GameSettingListModel::gameSettingValueChanged);
}
//
void GameSettingListModel::gameSettingValueChanged(const char* name) {
   item_type* item = nullptr;
   int i = 0;
   for (; i < this->children.size(); ++i) {
      auto* current = this->children[i];
      if (current->name.compare(name, Qt::CaseInsensitive) == 0) {
         item = current;
         break;
      }
   }
   dovah::loaded_game_setting loaded;
   auto& editor = DovahKitCore::get();
   if (editor.get_loaded_game_setting(name, loaded)) {
      if (item) {
         item->updateFrom(loaded);
         //
         auto root  = QModelIndex();
         auto start = this->index(i, 0, root);
         auto end   = this->index(i, this->columnCount(root) - 1, root);
         emit dataChanged(start, end);
      } else {
         auto* item = new item_type(loaded);
         //
         auto first_inserted = this->children.size();
         auto last_inserted  = first_inserted;
         this->beginInsertRows(QModelIndex(), first_inserted, last_inserted); // we're not passing the count, we're passing the index of the last row. how annoying.
         this->children.push_back(item);
         this->endInsertRows();
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
               return QObject::tr("%1%2").arg(item->name).arg(item->is_in_active_file ? QObject::tr(" *", "game setting window - active file setting marker") : "");
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
               switch (item->type) {
                  case dovah::game_setting_type::boolean:
                     return Qt::AlignCenter;
                  case dovah::game_setting_type::float32:
                  case dovah::game_setting_type::integer:
                     return Qt::AlignRight;
                  case dovah::game_setting_type::string:
                     return Qt::AlignLeft;
               }
               break;
         }
         break;
      case ColumnFormID:
         switch (role) {
            case Qt::DisplayRole:
               if (item->last_file.isEmpty() && !item->is_in_active_file)
                  return "";
               return QString("%1").arg(item->formID, 8, 16, QChar('0')).toUpper();
            case SortRole: // sorting
               return item->formID;
         }
         break;
      case ColumnFile:
         switch (role) {
            case Qt::DisplayRole:
               if (item->last_file.isEmpty()) {
                  if (item->is_in_active_file)
                     return QObject::tr("<new file>", "game setting list - untitled/implicit active file");
                  return QObject::tr("", "game setting list - no file");
               }
               [[fallthrough]];
            case SortRole: // sorting
            case FilterRole: // filtering
               return item->last_file;
         }
         break;
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
   auto& editor = DovahKitCore::get();
   auto& queued = this->queued_additions;
   for (auto& definition : dovah::game_settings) {
      dovah::loaded_game_setting loaded;
      if (editor.get_loaded_game_setting(definition.name, loaded)) {
         auto* item = new item_type(loaded);
         queued.push_back(item);
         if (loaded.source_file && editor.loaded_file_is_active(*loaded.source_file))
            item->is_in_active_file = true;
      } else {
         queued.push_back(new item_type(definition));
      }
   }
   editor.for_each_loaded_game_setting([this, &editor](const dovah::loaded_game_setting& loaded) {
      if (loaded.definition)
         return false; // continue
      auto* item = new item_type(loaded);
      this->queued_additions.push_back(item);
      if (loaded.source_file && editor.loaded_file_is_active(*loaded.source_file))
         item->is_in_active_file = true;
      return false; // continue
   });
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
   header->resizeSection(GameSettingListModel::ColumnName,   metrics.boundingRect("XMMX").width() * 1.5F + 4);
   header->resizeSection(GameSettingListModel::ColumnValue,  metrics.boundingRect("000.000000").width() * 1.5F + 4);
   header->resizeSection(GameSettingListModel::ColumnFormID, 4);
   header->resizeSection(GameSettingListModel::ColumnFile,   metrics.boundingRect("Skyrim.esm").width() * 1.5F + 4);
   header->setSectionResizeMode(GameSettingListModel::ColumnName,   QHeaderView::Stretch);
   header->setSectionResizeMode(GameSettingListModel::ColumnValue,  QHeaderView::Interactive);
   header->setSectionResizeMode(GameSettingListModel::ColumnFormID, QHeaderView::Interactive);
   header->setSectionResizeMode(GameSettingListModel::ColumnFile,   QHeaderView::Interactive);
   //
   QObject::connect(this, &QTableView::doubleClicked, [this](const QModelIndex& index) {
      auto* proxy = (QSortFilterProxyModel*)this->model();
      auto  real  = proxy->mapToSource(index); // the (index) we received is specific to the proxy; we need an index relative to the underlying model
      if (!real.isValid())
         return;
      auto data = (model_item_type*)real.internalPointer();
      if (!data)
         return;
      //
      // TODO: show the setting *and* focus the relevant editing control
      //
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