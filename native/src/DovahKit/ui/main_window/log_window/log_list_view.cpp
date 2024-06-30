#include "./log_list_view.h"
#include <QHeaderView>
#include <QLineEdit>
#include "helpers/qt/strings.h"
#include "widgets/DKHeaderView.h"
#include "editor/core.h"
#include "editor/open_window_for_form.h"
#include "editor/helpers/backend_error_to_string.h"
#include "editor/helpers/backend_warning_to_string.h"
#include "editor/subsystems/message_log/core.h"

#include "dovah/notices/base_form_load_warning.h"
#include "dovah/notices/base_form_save_error.h"

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

   using logging_subsystem = dovahkit::subsystems::message_log::core;

   auto& editor  = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAcquireComplete,     this, &LogListModel::dataAcquireComplete);
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent,     this, &LogListModel::dataAbandonImminent);
   QObject::connect(&editor, &DovahKitCore::dataSaveImminent,        this, &LogListModel::dataSaveImminent);
   QObject::connect(&editor, &DovahKitCore::dataSaveComplete,        this, &LogListModel::dataSaveComplete);
   QObject::connect(&editor, &DovahKitCore::formRenumbered,          this, &LogListModel::formRenumbered);
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent,    this, &LogListModel::formDeletionImminent);

   auto& logging = logging_subsystem::get_or_create();
   QObject::connect(&logging, &logging_subsystem::backendErrorReceived,   this, &LogListModel::errorReceived);
   QObject::connect(&logging, &logging_subsystem::backendWarningReceived, this, &LogListModel::warningReceived);
   QObject::connect(&logging, &logging_subsystem::logItemReceived, this, [this](const item_type& item) {
      this->addLogItem(item);
   });
}

void LogListModel::dataAcquireComplete() {
   int none_stubs = 0;
   DovahKitCore::get().for_each_form_of_type(dovah::form_type::none, [&none_stubs](dovah::form_stub* stub) {
      if (stub->is_none_stub())
         //
         // Some "legitimate" form stubs can be none-type, such as the Papyrus Persistence Form, 
         // so we have to actually check if the none-type stub is a none-stub.
         //
         ++none_stubs;
      return false;
   });
   if (none_stubs) {
      this->_createLogItem(
         tr("Forms in the loaded files contain dangling references to %1 non-existent form(s). Check the \"Missing\" category in the Object Window for a list of the missing forms' form IDs, and view the Use Info on entries to see what's referring to them. It's normal for official game files to have this problem.", "log window")
            .arg(none_stubs),
         ui::types::log_item_type::warning,
         ui::types::log_item_context::file_load
      );
   }
   this->_createLogItem(
      tr("All files have been loaded.", "log window"),
      ui::types::log_item_type::message,
      ui::types::log_item_context::file_load
   );
}
void LogListModel::dataAbandonImminent() {
   //
   // Don't batch warnings across load orders.
   //
   this->warnings_cause_by_form.clear();
}
void LogListModel::dataSaveImminent() {
   this->_createLogItem(tr("Saving active file...", "log window"));
}
void LogListModel::dataSaveComplete() {
   this->_createLogItem(tr("The active file has been successfully saved.", "log window"));
}
void LogListModel::formRenumbered(dovah::form_stub*, dovah::bare_form_id_t prior, dovah::bare_form_id_t after) {
   //
   // We suppress identical warnings for the same form. Ensure that we properly keep
   // track of forms being renumbered (since we use form IDs as our map key).
   //
   auto& store = this->warnings_cause_by_form;
   auto  it    = store.find(prior);
   if (it == store.end())
      return;

   auto value = std::move(*it);
   store.erase(it);
   store.insert(after, std::move(value));
}
void LogListModel::formDeletionImminent(dovah::form_stub* stub, bool just_being_flagged) {
   //
   // We suppress identical warnings for the same form. Ensure that if a new form is 
   // created with the same ID, and somehow ends up having the same problems, that we
   // do treat it as a separate form.
   //
   if (just_being_flagged)
      return;
   auto& store = this->warnings_cause_by_form;
   auto  it    = store.find(stub->formID);
   if (it == store.end())
      return;
   store.erase(it);
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

   auto* item = new item_type(notice);
   if (item->empty()) [[unlikely]] {
      delete item;
      return;
   }
   
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

   if (this->_has_matching_notice(form_id, text))
      //
      // We may receive warnings during on-demand loading of a form. If the user loads the
      // form, unloads it, and reloads it again, let's maybe avoid showing them duplicates
      // of the same warnings, yeah?
      //
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
      return {};
   const auto* item = (item_type*)index.internalPointer();
   switch (index.column()) {
      case Column::Context:
         if (role == Qt::DecorationRole) {
            switch (item->context) {
               case ui::types::log_item_context::file_load:
                  return this->_icons.contexts.file_load;
               case ui::types::log_item_context::file_save:
                  return this->_icons.contexts.file_save;
               case ui::types::log_item_context::form_load:
                  return this->_icons.contexts.form_load;
               case ui::types::log_item_context::form_save:
                  return this->_icons.contexts.form_load; // TODO: Differentiate
            }
         }
         if (role == Qt::ToolTipRole) {
            switch (item->context) {
               case ui::types::log_item_context::file_load:
                  return tr("Initial file load");
               case ui::types::log_item_context::file_save:
                  return tr("File save");
               case ui::types::log_item_context::form_load:
                  return tr("Form data full load");
               case ui::types::log_item_context::form_save:
                  return tr("Form data save");
            }
         }
         break;
      case Column::Type:
         if (role == Qt::DecorationRole) {
            switch (item->type) {
               case ui::types::log_item_type::error:
                  return this->_icons.error;
               case ui::types::log_item_type::warning:
                  return this->_icons.warning;
            }
         }
         if (role == Qt::ToolTipRole) {
            switch (item->type) {
               case ui::types::log_item_type::error:
                  return tr("Error");
               case ui::types::log_item_type::warning:
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
   return {};
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

void LogListModel::_appendLogItem(item_type* item) {
   auto first_inserted = this->children.size();
   auto last_inserted  = first_inserted;
   this->beginInsertRows({}, first_inserted, last_inserted);
   this->children.push_back(item);
   this->endInsertRows();
}

void LogListModel::addLogItem(item_type&& src) {
   auto* item = new item_type(std::move(src));
   this->_appendLogItem(item);
}
void LogListModel::addLogItem(const item_type& src) {
   auto* item = new item_type(src);
   this->_appendLogItem(item);
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