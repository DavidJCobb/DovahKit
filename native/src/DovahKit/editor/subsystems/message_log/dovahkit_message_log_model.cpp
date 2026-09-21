#include "./dovahkit_message_log_model.h"
#include "helpers/qt/strings.h"
#include "../../core.h"
#include "../../open_window_for_form.h"
#include "../../helpers/backend_error_to_string.h"
#include "../../helpers/backend_warning_to_string.h"
#include "./core.h"

#include "dovah/notices/base_form_load_warning.h"
#include "dovah/notices/base_form_save_error.h"

namespace {
   using logging_subsystem = dovahkit::subsystems::message_log::core;
}

namespace dovahkit::subsystems::message_log {
   model::model(QObject* parent) : QAbstractTableModel(parent) {
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
   }

   void model::initialize(cobb::passkey<model, core>) {
      auto& editor  = DovahKitCore::get();
      QObject::connect(&editor, &DovahKitCore::dataAcquireFailed,       this, &model::dataAcquireFailed);
      QObject::connect(&editor, &DovahKitCore::dataAcquireComplete,     this, &model::dataAcquireComplete);
      QObject::connect(&editor, &DovahKitCore::dataAbandonImminent,     this, &model::dataAbandonImminent);
      QObject::connect(&editor, &DovahKitCore::dataSaveImminent,        this, &model::dataSaveImminent);
      QObject::connect(&editor, &DovahKitCore::dataSaveComplete,        this, &model::dataSaveComplete);
      QObject::connect(&editor, &DovahKitCore::formRenumbered,          this, &model::formRenumbered);
      QObject::connect(&editor, &DovahKitCore::formDeletionImminent,    this, &model::formDeletionImminent);

      auto& logging = logging_subsystem::get_or_create();
      QObject::connect(&logging, &logging_subsystem::backendErrorReceived,   this, &model::errorReceived);
      QObject::connect(&logging, &logging_subsystem::backendWarningReceived, this, &model::warningReceived);
      QObject::connect(&logging, &logging_subsystem::logItemReceived, this, [this](const item_type& item) {
         this->addLogItem(item);
      });
   }

   #pragma region Handlers
      void model::dataAcquireFailed(QString text) {
         this->_createLogItem(
            tr("Failed to load data files due to the following error:\n\n%1", "log window").arg(text),
            ui::types::log_item_type::error,
            ui::types::log_item_context::file_load
         );
      }
      void model::dataAcquireComplete() {
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
      void model::dataAbandonImminent() {
         //
         // Don't batch warnings across load orders.
         //
         this->warnings_cause_by_form.clear();
      }
      void model::dataSaveImminent() {
         this->_createLogItem(tr("Saving active file...", "log window"));
      }
      void model::dataSaveComplete() {
         QString text;
         auto    fn = DovahKitCore::get().get_active_file_name();
         if (fn.isEmpty()) {
            text = tr("The active file has been successfully saved.", "log window");
         } else {
            text = tr("The active file, %1, has been successfully saved.", "log window").arg(fn);
         }
         this->_createLogItem(text);
      }
      void model::formRenumbered(dovah::form_stub*, dovah::bare_form_id_t prior, dovah::bare_form_id_t after) {
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
      void model::formDeletionImminent(dovah::form_stub* stub, bool just_being_flagged) {
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

      bool model::_has_matching_notice(dovah::bare_form_id_t form_id, QString text) const {
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

      void model::errorReceived(const dovah::notices::base_error& notice) {
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
      void model::warningReceived(const dovah::notices::base_warning& notice) {
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
         ++this->_cached.warning_count;
         //
         this->endInsertRows();
      }
   #pragma endregion

   #pragma region QAbstractTableModel overrides
      /*virtual*/ QModelIndex model::index(int row, int column, const QModelIndex& parent) const /*override*/ {
         if (!this->hasIndex(row, column, parent))
            return {};
         item_type* childItem = this->children.value(row);
         if (childItem)
            return this->createIndex(row, column, childItem);
         return {};
      }
      /*virtual*/ QModelIndex model::parent(const QModelIndex& index) const /*override*/ {
         return {};
      }
      /*virtual*/ int model::rowCount(const QModelIndex& parent) const /*override*/ {
         if (parent.column() > 0)
            return 0;
         return this->children.size();
      }
      /*virtual*/ int model::columnCount(const QModelIndex& item) const /*override*/ {
         return ColumnCount;
      }
      /*virtual*/ Qt::ItemFlags model::flags(const QModelIndex& index) const /*override*/ {
         if (!index.isValid())
            return Qt::NoItemFlags;
         return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
      }
      /*virtual*/ QVariant model::data(const QModelIndex& index, int role) const /*override*/ {
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
   
      /*virtual*/ QVariant model::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
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
   #pragma endregion

   inline const model::item_type* model::row(int rowIndex) const noexcept {
      return this->children.value(rowIndex);
   }

   void model::_appendLogItem(item_type* item) {
      auto first_inserted = this->children.size();
      auto last_inserted  = first_inserted;
      this->beginInsertRows({}, first_inserted, last_inserted);
      this->children.push_back(item);
      if (item->type == ui::types::log_item_type::warning) {
         ++this->_cached.warning_count;
      }
      this->endInsertRows();
   }

   void model::addLogItem(item_type&& src) {
      auto* item = new item_type(std::move(src));
      this->_appendLogItem(item);
   }
   void model::addLogItem(const item_type& src) {
      auto* item = new item_type(src);
      this->_appendLogItem(item);
   }
   void model::clear() {
      this->beginResetModel();
      for (auto* item : this->children)
         delete item;
      this->children.clear();
      this->warnings_cause_by_form.clear();
      this->_cached.warning_count = 0;
      this->endResetModel();
   }
}