#include "./log_list_view.h"
#include <QHeaderView>
#include <QLineEdit>
#include "helpers/qt/strings.h"
#include "widgets/DKHeaderView.h"
#include "editor/core.h"
#include "editor/open_window_for_form.h"
#include "editor/helpers/backend_error_to_string.h"
#include "editor/helpers/backend_warning_to_string.h"

#include "dovah/notices/base_file_load_error.h"
#include "dovah/notices/base_file_load_warning.h"
#include "dovah/notices/base_form_load_warning.h"
#include "dovah/notices/base_form_save_error.h"
#include "dovah/notices/base_form_save_warning.h"

#pragma region LogListModelItem
LogListModelItem::LogListModelItem(const QString& t) {
   this->type = type_t::text;
   this->text = t;
}
LogListModelItem::LogListModelItem(const dovah::notices::base_error& notice) {
   this->metadata.type = Type::Error;
   this->text = editor_helpers::backend_error_to_string(notice);

   if (auto* casted = dynamic_cast<const dovah::notices::base_form_save_error*>(&notice)) {
      this->metadata.context = Context::FormSave;
   } else if (auto* casted = dynamic_cast<const dovah::notices::base_file_load_error*>(&notice)) {
      this->metadata.context = Context::FileLoad;
      this->file = QString::fromUtf8(QByteArray::fromStdString(casted->filename));
   }
}
LogListModelItem::LogListModelItem(const dovah::notices::base_warning& notice) {
   this->metadata.type = Type::Warning;
   this->text = editor_helpers::backend_warning_to_string(notice);

   if (auto* casted = dynamic_cast<const dovah::notices::base_form_load_warning*>(&notice)) {
      this->metadata.context = Context::FormLoad;
      this->file = QString::fromUtf8(QByteArray::fromStdString(casted->record_info.source_file));
   } else if (auto* casted = dynamic_cast<const dovah::notices::base_form_save_warning*>(&notice)) {
      this->metadata.context = Context::FormSave;
   } else if (auto* casted = dynamic_cast<const dovah::notices::base_file_load_warning*>(&notice)) {
      this->metadata.context = Context::FileLoad;
      this->file = QString::fromUtf8(QByteArray::fromStdString(casted->source_file));
   }
}

bool LogListModelItem::empty() const noexcept {
   return this->text.isEmpty();
}
#pragma endregion

#pragma region LogListModel
LogListModel::LogListModel(QObject* parent) : QAbstractTableModel(parent) {
   {
      auto& icon = this->_icons.error;
      icon.addFile(":/icons/log-window-icons/error-16.png", { 16, 16 });
      icon.addFile(":/icons/log-window-icons/error-64.png", { 64, 64 });
   }
   {
      auto& icon = this->_icons.warning;
      icon.addFile(":/icons/log-window-icons/warning-16.png", { 16, 16 });
      icon.addFile(":/icons/log-window-icons/warning-64.png", { 64, 64 });
   }
   {
      auto& icon = this->_icons.contexts.file_load;
      icon.addFile(":/icons/log-window-icons/context-file-load-16.png", { 16, 16 });
      icon.addFile(":/icons/log-window-icons/context-file-load-64.png", { 64, 64 });
   }
   {
      auto& icon = this->_icons.contexts.file_save;
      icon.addFile(":/icons/log-window-icons/context-file-save-16.png", { 16, 16 });
      icon.addFile(":/icons/log-window-icons/context-file-save-64.png", { 64, 64 });
   }
   {
      auto& icon = this->_icons.contexts.form_load;
      icon.addFile(":/icons/log-window-icons/context-form-16.png", { 16, 16 });
      icon.addFile(":/icons/log-window-icons/context-form-64.png", { 64, 64 });
   }

   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAcquireComplete,     this, &LogListModel::dataAcquireComplete);
   QObject::connect(&editor, &DovahKitCore::dataSaveImminent,        this, &LogListModel::dataSaveImminent);
   QObject::connect(&editor, &DovahKitCore::dataSaveComplete,        this, &LogListModel::dataSaveComplete);

   QObject::connect(&editor, &DovahKitCore::backendErrorReceived,   this, &LogListModel::errorReceived);
   QObject::connect(&editor, &DovahKitCore::backendWarningReceived, this, &LogListModel::warningReceived);
}

void LogListModel::dataAcquireComplete() {
   int none_stubs = 0;
   DovahKitCore::get().for_each_form_of_type(dovah::form_type::none, [&none_stubs](dovah::form_stub* stub) {
      if (stub->is_none_stub())
         ++none_stubs;
      return false;
   });
   if (none_stubs) {
      this->addTextEntry(
         tr("Forms in the loaded files contain dangling references to %1 non-existent form(s). Check the \"Missing\" category in the Object Window for a list of the missing forms' form IDs, and view the Use Info on entries to see what's referring to them. It's normal for official game files to have this problem.", "log window")
            .arg(none_stubs)
      );
   }
   //
   this->addTextEntry(tr("All files have been loaded.", "log window"));
}
void LogListModel::dataSaveImminent() {
   this->addTextEntry(tr("Saving active file...", "log window"));
}
void LogListModel::dataSaveComplete() {
   this->addTextEntry(tr("The active file has been successfully saved.", "log window"));
}

bool LogListModel::_has_matching_notice(dovah::bare_form_id_t form_id, QString text) const {
   if (!form_id)
      return false;

   auto& map = this->warnings_cause_by_form;
   auto it = map.find(form_id);
   if (it == map.end())
      return false;

   auto& list = *it;
   for (auto* item : list) {
      if (item->text == text)
         return true;
   }

   return false;
}

void LogListModel::errorReceived(const dovah::notices::base_error& notice) {
   dovah::bare_form_id_t form_id = 0;

   if (auto* casted = dynamic_cast<const dovah::notices::base_form_save_error*>(&notice)) {
      form_id = casted->subject.formID;
   }

   auto text = editor_helpers::backend_error_to_string(notice);
   if (text.isEmpty())
      return;

   auto* item = new item_type(notice);
   
   auto first_inserted = this->children.size();
   auto last_inserted  = first_inserted;
   this->beginInsertRows({}, first_inserted, last_inserted);
   //
   this->children.push_back(item);
   if (form_id) {
      auto& list = this->warnings_cause_by_form[form_id];
      list.push_back(item);
   }
   //
   this->endInsertRows();
}
void LogListModel::warningReceived(const dovah::notices::base_warning& notice) {
   dovah::bare_form_id_t form_id = 0;

   if (auto* casted = dynamic_cast<const dovah::notices::base_form_load_warning*>(&notice)) {
      if (!casted->record_info.is_winning_record)
         return;
      form_id = casted->subject.formID;
   }

   auto text = editor_helpers::backend_warning_to_string(notice);
   if (text.isEmpty())
      return;
   /*//
   //
   // TODO: This doesn't account for "load order" boundaries: if the same warning text 
   //       and form ID occur across different load orders, then the latter would be lost.
   //
   //       Do we really need this?
   //
   if (this->_has_matching_notice(form_id, text))
      return;
   //*/

   auto* item = new item_type(notice);
   
   auto first_inserted = this->children.size();
   auto last_inserted  = first_inserted;
   this->beginInsertRows({}, first_inserted, last_inserted);
   //
   this->children.push_back(item);
   if (form_id) {
      auto& list = this->warnings_cause_by_form[form_id];
      list.push_back(item);
   }
   //
   this->endInsertRows();
}

QModelIndex LogListModel::index(int row, int column, const QModelIndex& parent) const {
   if (!this->hasIndex(row, column, parent))
      return QModelIndex();
   item_type* childItem = this->children.value(row);
   if (childItem)
      return this->createIndex(row, column, childItem);
   return QModelIndex();
}
QModelIndex LogListModel::parent(const QModelIndex& index) const {
   return QModelIndex();
}
int LogListModel::rowCount(const QModelIndex& parent) const {
   if (parent.column() > 0)
      return 0;
   return this->children.size();
}
int LogListModel::columnCount(const QModelIndex& item) const {
   return ColumnCount;
}
Qt::ItemFlags LogListModel::flags(const QModelIndex& index) const {
   if (!index.isValid())
      return Qt::NoItemFlags;
   return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
}
QVariant LogListModel::data(const QModelIndex& index, int role) const {
   if (!index.isValid())
      return QVariant();
   auto item   = (item_type*)index.internalPointer();
   auto column = index.column();
   switch (column) {
      case Column::Context:
         if (role == Qt::DecorationRole) {
            switch (item->metadata.context) {
               case LogListModelItem::Context::FileLoad:
                  return this->_icons.contexts.file_load;
               case LogListModelItem::Context::FileSave:
                  return this->_icons.contexts.file_save;
               case LogListModelItem::Context::FormLoad:
                  return this->_icons.contexts.form_load;
               case LogListModelItem::Context::FormSave:
                  return this->_icons.contexts.form_load; // TODO: Differentiate
            }
         }
         if (role == Qt::ToolTipRole) {
            switch (item->metadata.context) {
               case LogListModelItem::Context::FileLoad:
                  return tr("Initial file load");
               case LogListModelItem::Context::FileSave:
                  return tr("File save");
               case LogListModelItem::Context::FormLoad:
                  return tr("Form data full load");
               case LogListModelItem::Context::FormSave:
                  return tr("Form data save");
            }
         }
         break;
      case Column::Type:
         if (role == Qt::DecorationRole) {
            switch (item->metadata.type) {
               case LogListModelItem::Type::Error:
                  return this->_icons.error;
               case LogListModelItem::Type::Warning:
                  return this->_icons.warning;
            }
         }
         if (role == Qt::ToolTipRole) {
            switch (item->metadata.type) {
               case LogListModelItem::Type::Error:
                  return tr("Error");
               case LogListModelItem::Type::Warning:
                  return tr("Warning");
            }
         }
         break;
      case Column::Text:
         if (role == Qt::DisplayRole)
            return item->text;
         break;
      case Column::File:
         if (role == Qt::DisplayRole || role == Qt::ToolTipRole)
            return item->file;
         break;
   }
   return QVariant();
}
inline const LogListModel::item_type* LogListModel::row(int rowIndex) const noexcept {
   return this->children.value(rowIndex);
}
//
QVariant LogListModel::headerData(int section, Qt::Orientation orientation, int role) const {
   if (orientation != Qt::Orientation::Horizontal)
      return {};
   switch (role) {
      case Qt::DisplayRole:
      case Qt::ToolTipRole:
         switch (section) {
            case Column::Type:    return tr("Type",    "log window header");
            case Column::Context: return tr("Context", "log window header");
            case Column::Text:    return tr("Text",    "log window header");
            case Column::File:    return tr("File",    "log window header");
         }
         break;
   }
   return {};
}

void LogListModel::addTextEntry(const QString& text) {
   auto* item = new item_type(text);
   //
   auto first_inserted = this->children.size();
   auto last_inserted  = first_inserted;
   this->beginInsertRows(QModelIndex(), first_inserted, last_inserted);
   this->children.push_back(item);
   this->endInsertRows();
}
void LogListModel::clear() {
   this->beginResetModel();
   for (auto* item : this->children)
      delete item;
   this->children.clear();
   this->warnings_cause_by_form.clear();
   this->endResetModel();
}
#pragma endregion

#pragma region LogList
LogList::LogList(QWidget* parent) : QTableView(parent) {
   this->setModel(new model_type(this));

   auto metrics = QFontMetrics(this->font());

   if (auto* vh = this->verticalHeader()) {
      vh->setDefaultSectionSize(metrics.height()); // nix the janky padding QTableView adds to rows by default (wow! what a good widget!)
   }

   constexpr const size_t icon_size = 16;

   this->setIconSize({ icon_size, icon_size });

   auto* header = new DKHeaderView(Qt::Orientation::Horizontal, this);
   header->setFlexResizeEnabled(true);
   this->setHorizontalHeader(header);

   header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignBaseline);
   header->setMinimumSectionSize(2);
   header->setColumnFlex(LogListModel::Column::Type,    0, 0, icon_size + 8);
   header->setColumnFlex(LogListModel::Column::Context, 0, 0, icon_size + 8);
   header->setColumnFlex(LogListModel::Column::Text,    1, 1, 2);
   header->setColumnFlex(LogListModel::Column::File,    0, 0, metrics.boundingRect("Dragonborn.esm").width() * 1.5F + 4);
   header->setSectionResizeMode(LogListModel::Column::Text, QHeaderView::Interactive);
   header->setSectionResizeMode(LogListModel::Column::File, QHeaderView::Interactive);
   header->setStretchLastSection(false);
};
#pragma endregion