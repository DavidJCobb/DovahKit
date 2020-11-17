#include "core.h"
#include <QFile>
#include <QInputDialog>
#include <QMessageBox>
#include <QProcessEnvironment>
#include <QTextCodec>
#include <QThread>
#include "../helpers/performance.h"
#include "../helpers/windows_registry.h"
#include "../dovah/form_stub.h"
#include "../dovah/notice_code_list.h"
#include "../dovah/localized_strings.h"
#include "../dovah/files/bsa/bsa_load_order.h"
#include "../dovah/files/tes_file_reading/file.h"
#include "../dovah/utils/get_user_language_name.h"
#include "../dovah/forms/DefaultObjectManager.h"
#include "core_internals/load_task.h"
#include "core_internals/read_warning_dispatcher.h"
#include "helpers/make_editor_id_for_duplicate.h"
#include "../ui/main_window/delete_form_dialog.h"
#include <QDebug>

namespace {
   void _on_form_created(dovah::form_stub* stub) {
      if (stub)
         emit DovahKitCore::get().formCreated(stub);
   }
   void _on_form_loss(dovah::form_stub& stub) {
      emit DovahKitCore::get().formDeletionImminent(&stub, false);
      emit DovahKitCore::get().formDeletionComplete(stub.formID, false);
   }
   void _on_form_renumber(dovah::form_stub& stub, dovah::bare_form_id_t oldID, dovah::bare_form_id_t newID) {
      emit DovahKitCore::get().formRenumbered(&stub, oldID, newID);
   }
   void _on_mass_renumber() {
      emit DovahKitCore::get().formsRenumberedEnMasse();
   }
   void _on_read_warning(const dovah::file_read_warning& warning) {
      //
      // This callback can come from the initial file load (where form stubs are built), or 
      // when loading the full contents of a form. This particular frontend runs the initial 
      // file load on a worker thread to avoid blocking the UI, which means that we need to 
      // adapt the data sent by this callback and guarantee that the signal we emit goes to 
      // the main thread.
      //
      // Accordingly, we rely on a "dispatcher" singleton that: wraps the warning struct in 
      // another struct suitable for use as a Qt metatype; and then emits a signal, which 
      // DovahKitCore will in turn listen for.
      //
      // (If we were to just emit the DovahKitCore signal from here, without registering a 
      // metatype, then it would only trigger slots registered on whatever thread we're 
      // emitting from. Registering the metatype allows Qt to copy the data and trigger 
      // slots across threads.)
      //
      DovahKitEditorInternals::read_warning_dispatcher::get().send(warning);
   }
}
DovahKitCore::DovahKitCore() {
   qRegisterMetaType<file_load_stats>(); // needed so that QObject::connect can pass these across threads (by copying them)
   //
   this->load_order = new dovah::file_load_order;
   this->_configure_load_order();
   //
   this->set_encoding();
   //
   {
      using dispatcher_t = DovahKitEditorInternals::read_warning_dispatcher;
      dispatcher_t& dispatcher = dispatcher_t::get();
      QObject::connect(&dispatcher, &dispatcher_t::received, this, [this](DovahKitEditorInternals::multithreadable_file_read_warning w) {
         emit this->fileLoadWarningReceived(w.warning);
      }, Qt::QueuedConnection);
   }
}
DovahKitCore::~DovahKitCore() {
   if (auto thread = this->async_loader) {
      thread->quit();
      thread->wait();
      delete thread;
      this->async_loader = nullptr;
   }
   //
   delete this->load_order;
   this->load_order = nullptr;
}
void DovahKitCore::_configure_load_order() {
   this->load_order->on_form_create   = &_on_form_created;
   this->load_order->on_form_loss     = &_on_form_loss;
   this->load_order->on_form_renumber = &_on_form_renumber;
   this->load_order->on_mass_renumber = &_on_mass_renumber;
   this->load_order->on_read_warning  = &_on_read_warning;
   this->load_order->adopt_archive_list(*(new dovah::bsa_load_order));
}
void DovahKitCore::abandon_data() {
   emit dataAbandonImminent();
   delete this->load_order;
   this->loaded     = false;
   this->load_order = new dovah::file_load_order;
   this->_configure_load_order();
   emit dataAbandonComplete();
}
void DovahKitCore::set_load_order_folder(const std::filesystem::path& p) {
   this->load_order->base_path = p.string();
}
void DovahKitCore::queue_load_order_file(const std::filesystem::path& p) {
   this->load_order->queue_file(p.string());
}
void DovahKitCore::unqueue_load_order_file(const std::filesystem::path& p) {
   this->load_order->unqueue_file(p.string());
}
void DovahKitCore::set_load_queued_game(dovah::game g) {
   this->load_order->change_current_game(g);
}
void DovahKitCore::set_queued_active_file(const std::filesystem::path& p) {
   this->load_order->queue_active_file(p.string());
}

bool DovahKitCore::acquire_load_order_data(bool async) {
   if (this->loading || this->async_loader)
      return false;
   this->loading = true;
   if (async) {
      auto worker = new DovahKitEditorInternals::load_task(*this);
      auto thread = this->async_loader;
      if (!thread)
         thread = this->async_loader = new QThread;
      worker->moveToThread(thread);
      QObject::connect(thread, &QThread::started,  worker, &DovahKitEditorInternals::load_task::exec);
      QObject::connect(thread, &QThread::finished, worker, &QObject::deleteLater);
      QObject::connect(thread, &QThread::finished, this, [this, thread]() {
         this->async_loader = nullptr;
         this->loading      = false;
         thread->deleteLater();
      });
      //
      // NOTE: For any signals emitted by the worker and received from the spawning thread, you MUST 
      // specify a context object (i.e. the QObject before your functor). If you don't, Qt WILL fail 
      // an assertion when the signal is received, before even executing any of the code in your 
      // signal handler, and the assertion message WILL be completely wrong and waste multiple hours 
      // of your goddamned time.
      //
      // Presumably it has something to do with Qt::AutoConnection, which is supposed to adapt signals 
      // across threads; I assume it can't do that if you don't explicitly provide a context object 
      // for it to adapt to. (All QObjects are aware of their owning thread, apparently.)
      // 
      // Naturally, none of this is mentioned in their documentation or examples for QThread, at least 
      // as of this writing. It's far from the only thing missing, either.
      //
      QObject::connect(worker, &DovahKitEditorInternals::load_task::complete, this, [this](file_load_stats stats) {
         emit dataAcquireComplete();
         emit fileLoadStatisticsAvailable(stats);
      });
      QObject::connect(worker, &DovahKitEditorInternals::load_task::failed, this, [this]() {
         emit dataAcquireFailed(this->load_order->load_error);
      });
      QObject::connect(worker, &DovahKitEditorInternals::load_task::ended, this, [this, thread]() {
         //
         // As a bonus, we also have to call QThread::quit() manually when our work is complete. For 
         // some reason, QThread isn't cognizant of its no longer having any work to do. The documen-
         // tation for QThread does not mention this, and the examples actively omit it. I managed to 
         // find it explained over at <https://stackoverflow.com/a/17094375>, which has exactly the 
         // level of detail that you should be able to expect from the official documentation, but 
         // can't.
         //
         thread->quit();
      });
      //
      thread->start();
      return false;
   }
   auto task = DovahKitEditorInternals::load_task(*this);
   task.exec();
   if (task.result) {
      emit dataAcquireComplete();
      emit fileLoadStatisticsAvailable(task.stats);
   } else {
      emit dataAcquireFailed(this->load_order->load_error);
   }
   return task.result;
}

dovah::game DovahKitCore::get_current_game() const noexcept {
   return this->load_order->get_current_game();
}

float DovahKitCore::assess_load_progress() const noexcept {
   return this->load_order->assess_load_progress();
}
const dovah::file_read_error& DovahKitCore::get_last_read_error() const noexcept {
   return this->load_order->load_error;
}

std::vector<const dovah::tes_file_reading::file_reader*> DovahKitCore::get_loaded_files() const noexcept {
   return this->load_order->get_loaded_files();
}
bool DovahKitCore::loaded_file_is_active(const dovah::tes_file_reading::file_reader& file) const noexcept {
   return this->load_order->file_is_active(file);
}
bool DovahKitCore::active_file_has_name() const noexcept {
   return this->load_order->active_file_has_name();
}
QString DovahKitCore::get_active_file_name() const noexcept {
   std::filesystem::path out;
   this->load_order->get_active_file_name(out);
   return QString::fromStdWString(out.wstring());
}
bool DovahKitCore::has_active_file() const noexcept {
   return this->load_order->has_active_file();
}
bool DovahKitCore::save_active_file(std::filesystem::path name_to_use_if_nameless, const dovah::tes_file_writing::write_config& cfg) {
   //
   // TODO: fail if a save is in progress.
   //
   emit dataSaveImminent();
   if (this->load_order->save_active_file(name_to_use_if_nameless, cfg)) {
      emit dataSaveComplete();
      return true;
   }
   emit dataSaveFailed(this->load_order->save_error);
   return false;
}
QString DovahKitCore::get_active_file_author() const noexcept {
   if (auto* header = this->load_order->get_active_file_header())
      return QString::fromStdString(header->author);
   return QString();
}
QString DovahKitCore::get_active_file_description() const noexcept {
   if (auto* header = this->load_order->get_active_file_header())
      return QString::fromStdString(header->description);
   return QString();
}
void DovahKitCore::set_active_file_author(const QString& text) const noexcept {
   if (auto* header = this->load_order->get_active_file_header())
      header->author = text.toStdString();
}
void DovahKitCore::set_active_file_description(const QString& text) const noexcept {
   if (auto* header = this->load_order->get_active_file_header())
      header->description = text.toStdString();
}
const dovah::tes_file_header* DovahKitCore::get_active_file_header() const noexcept {
   return this->load_order->get_active_file_header();
}

bool DovahKitCore::for_each_load_order_filename(std::function<bool(std::filesystem::path, bool is_active_file)> functor) const noexcept {
   return this->load_order->for_each_load_order_filename(functor);
}
bool DovahKitCore::load_order_has_file(const std::filesystem::path& filename, bool ignore_if_active_file) const noexcept {
   if (ignore_if_active_file)
      return this->load_order->has_non_active_file(filename);
   return this->load_order->has_file(filename);
}

const dovah::file_write_error& DovahKitCore::get_last_write_error() const noexcept {
   return this->load_order->save_error;
}
const dovah::file_write_warning& DovahKitCore::get_write_warning() const noexcept {
   return this->load_order->save_warning;
}

uint32_t DovahKitCore::count_forms_of_type(form_type_t ft) const noexcept {
   return this->load_order->count_forms_of_type(ft);
}
dovah::form_stub* DovahKitCore::get_form(bare_form_id_t formID) const noexcept {
   return this->load_order->get_form(formID);
}
dovah::form_stub* DovahKitCore::get_form(form_type_t ft, bare_form_id_t formID) const noexcept {
   return this->load_order->get_form(ft, formID);
}
dovah::form_stub* DovahKitCore::get_form_of_probable_type(form_type_t ft, bare_form_id_t formID) const noexcept {
   return this->load_order->get_form_of_probable_type(ft, formID);
}
dovah::form_stub* DovahKitCore::get_singleton_form(form_type_t ft, bool create_if_missing) const noexcept {
   if (this->load_order)
      return this->load_order->get_canonical_instance_of_singleton_form(ft, create_if_missing);
   return nullptr;
}
bool DovahKitCore::for_each_form(std::function<bool(dovah::form_stub*)> functor) {
   for (uint8_t i = 0; i < dovah::form_types.size(); ++i) {
      if (this->load_order->for_each_form_of_type(i, functor))
         return true;
   }
   return false;
}
bool DovahKitCore::for_each_form_of_type(form_type_t ft, std::function<bool(dovah::form_stub*)> functor) {
   return this->load_order->for_each_form_of_type(ft, functor);
}
bool DovahKitCore::for_each_impossible_to_save_form(dovah::game g, std::function<bool(dovah::form_stub*)> functor) {
   return this->load_order->for_each_impossible_to_save_form(g, functor);
}

bool DovahKitCore::is_form_defined_in_active_file(dovah::form_stub* stub) const noexcept {
   if (!stub)
      return false;
   return this->load_order->is_defined_in_active_file(*stub);
}

dovah::form_stub* DovahKitCore::create_form_of_type(dovah::form_type_t ft) {
   if (!this->loaded)
      return nullptr;
   auto* stub = this->load_order->create_form_of_type(ft);
   return stub;
}
dovah::form_creation_request DovahKitCore::request_form_creation(dovah::form_type_t ft) noexcept {
   return this->load_order->request_form_creation(ft);
}
dovah::form_duplication_request DovahKitCore::request_form_duplication() noexcept {
   return this->load_order->request_form_duplication();
}
dovah::form_renumber_request DovahKitCore::request_form_renumber(dovah::form_stub& stub, bare_form_id_t desiredID) noexcept {
   return this->load_order->request_form_renumber(stub, desiredID);
}

namespace {
   void _report_duplicate_form_error(QWidget* parent, const QString& text) {
      QMessageBox::critical(
         parent,
         QObject::tr("Error", "create new form error"),
         QObject::tr("Unable to duplicate this form. %1").arg(text)
      );
   }
   QString _stringify_duplicate_form_a_posteriori_error(dovah::notice_code_t code) {
      using notice_code = dovah::notice_code;
      switch (code) {
         case notice_code::unknown_form_type:
            return QObject::tr("An internal program error occurred: DovahKit tried to create a form but supplied a bad form type.");
         case notice_code::no_active_file:
            //
            // This should've been caught beforehand.
            //
            break;
         case notice_code::form_id_unavailable_for_new_form:
            //
            // This should've been caught beforehand.
            //
            break;
         case notice_code::unimplemented_form_type:
            return QObject::tr("DovahKit does not support editing this form type.");
         case notice_code::invalid_parent_child_relationship:
            return QObject::tr("The specified parent form cannot have a child form of this type.");
         case notice_code::exterior_grid_coordinates_already_taken:
            return QObject::tr("The specified worldspace already has an exterior cell at the desired grid coordinates.");
         case notice_code::cannot_create_reference_with_no_parent_cell:
            return QObject::tr("References cannot be created outside of a cell.");
         case notice_code::interior_cell_clone_cannot_have_parent:
            return QObject::tr("Interior cells cannot have a parent worldspace.");
         case notice_code::exterior_cell_clone_must_have_parent:
            return QObject::tr("Exterior cells must have a parent worldspace.");
      }
      return "";
   }
}
dovah::form_stub* DovahKitCore::duplicate_form(dovah::form_stub& original, QWidget* dialog_parent) {
   auto  request = this->request_form_duplication();
   request.set_target(&original);
   if (request.has_error()) {
      auto errors = request.get_error_codes();
      //
      unsigned int needed_forms    = request.get_total_form_count();
      unsigned int no_reservations = 0;
      unsigned int other_failures  = 0;
      //
      for (auto e : errors) {
         if (e == dovah::notice_code::none)
            continue;
         if (e == dovah::notice_code::no_active_file) {
            _report_duplicate_form_error(dialog_parent, tr("There is neither an active file in the load order nor any room in the load order for a new file."));
            return nullptr;
         }
         if (e == dovah::notice_code::form_id_unavailable_for_new_form) {
            ++no_reservations;
            continue;
         }
         ++other_failures;
      }
      auto    total = no_reservations + other_failures;
      QString text;
      if (total > 1) {
         if (no_reservations) {
            text = tr("A total of %1 forms need to be created, but the file only has %2 free form IDs to spare.").arg(needed_forms).arg(no_reservations);
            if (other_failures)
               text += tr(" Additional errors were encountered while validating %3 other forms.").arg(other_failures);
            text += tr(" No forms were created.");
         } else {
            text = tr("A total of %1 forms need to be created, but errors were encountered while validating %2 of those forms.").arg(needed_forms).arg(other_failures);
            text += tr(" No forms were created.");
         }
      } else {
         if (no_reservations) {
            text = tr("The file doesn't have any free form IDs that a new form could use.");
         } else {
            text = tr("An error occurred.");
         }
      }
      _report_duplicate_form_error(dialog_parent, text);
      return nullptr;
   }
   //
   if (!dovah::form_type_info::form_type_is_reference(original.formType)) { // shouldn't ever happen for the Object Window, but eh
      QString suggestion = editor_helpers::make_editor_id_for_duplicate(original.get_editor_id());
      bool    ok         = false;
      QString editor_id  = QInputDialog::getText(dialog_parent, tr("Set editor ID"), tr("Editor ID:"), QLineEdit::Normal, suggestion, &ok);
      if (!ok)
         return nullptr;
      request.editorID = editor_id.toStdString();
   }
   auto result = request.commit();
   //
   QString error_text;
   auto    main_error   = request.get_main_form_error_code();
   auto    child_errors = request.get_child_form_error_codes();
   if (main_error != dovah::default_notice_code) {
      error_text = _stringify_duplicate_form_a_posteriori_error(main_error);
   }
   if (!child_errors.empty()) {
      if (error_text.isEmpty())
         error_text = tr("The following errors were encountered:<br/><br/>");
      else
         error_text += tr(" Additionally, the following errors were encountered when trying to clone this form's children:<br/><br/>");
      //
      for (auto e : child_errors) {
         error_text += _stringify_duplicate_form_a_posteriori_error(e);
      }
   }
   if (!error_text.isEmpty()) {
      _report_duplicate_form_error(dialog_parent, error_text);
   }
   return result;
}

void DovahKitCore::delete_form(dovah::form_stub& target, QWidget* dialog_parent) {
   auto request = this->load_order->request_form_deletion(target);
   auto result  = request.get_result_code();
   if (result != dovah::form_deletion_request::result_code::pending) {
      using result_code = dovah::form_deletion_request::result_code;
      //
      QString text;
      switch (result) {
         case result_code::error_cannot_delete_hardcoded_form:
            text = tr("The form is hardcoded into the game engine and cannot be deleted.");
            break;
         case result_code::error_cannot_load_form:
            text = tr("DovahKit doesn't currently support this form type, which means that it cannot flag the form as deleted.");
            break;
         case result_code::error_cannot_load_user:
            text = tr("One of the forms that uses this form is of an unsupported type, which means that that use cannot be severed.");
            break;
      }
      QMessageBox::critical(
         dialog_parent,
         QObject::tr("Error", "delete form error"),
         QObject::tr("Unable to delete this form. %1").arg(text)
      );
      return;
   }
   if (dialog_parent) {
      //
      // Show a confirmation prompt.
      //
      auto* confirm = new DeleteFormDialog(dialog_parent);
      confirm->updateFromDeletionRequest(request);
      auto  result = confirm->exec();
      delete confirm;
      if (result == QDialog::Rejected)
         return;
   }
   //
   struct _entry {
      dovah::bare_form_id_t id;
      bool flagged;
   };
   std::vector<_entry> formIDs;
   auto forms_d = request.get_forms_pending_delete(false);
   auto forms_f = request.get_forms_pending_flagging();
   formIDs.reserve(forms_d.size() + forms_f.size());
   for (auto* stub : forms_d) {
      emit this->formDeletionImminent(stub, false);
      formIDs.push_back({ stub->formID, false });
   }
   for (auto* stub : forms_f) {
      emit this->formDeletionImminent(stub, true);
      formIDs.push_back({ stub->formID, true });
   }
   //
   request.commit();
   //
   result = request.get_result_code();
   if (result != dovah::form_deletion_request::result_code::success) {
      using result_code = dovah::form_deletion_request::result_code;
      //
      QString text;
      QMessageBox::critical(
         dialog_parent,
         QObject::tr("Error", "delete form error"),
         QObject::tr("Unable to delete this form. %1").arg(text)
      );
      return;
   }
   //
   for (auto& entry : formIDs)
      emit this->formDeletionComplete(entry.id, entry.flagged);
}

bool DovahKitCore::get_loaded_game_setting(const char* name, dovah::loaded_game_setting& out) {
   if (!this->load_order)
      return false;
   return this->load_order->get_loaded_setting_by_name(name, out);
}
bool DovahKitCore::for_each_loaded_game_setting(std::function<bool(const dovah::loaded_game_setting&)> callback) {
   if (!this->load_order)
      return false;
   return this->load_order->for_each_loaded_game_setting(callback);
}
bool DovahKitCore::edit_game_setting(const char* name, const dovah::game_setting_value& value) {
   if (!this->load_order)
      return false;
   auto request = this->load_order->request_game_setting_change();
   request.setting.name  = name;
   request.setting.value = value;
   request.commit();
   if (request.was_successful()) {
      emit this->gameSettingValueChanged(name);
   } else {
      emit this->gameSettingValueChangeFailed(name, request.get_notice_code());
   }
   return request.was_successful();
}
//
namespace {
   void _report_game_setting_renumber_error(QWidget* parent, const QString& text) {
      QMessageBox::critical(
         parent,
         QObject::tr("Error", "renumber GMST error"),
         QObject::tr("Unable to change this game setting's form ID. %1").arg(text)
      );
   }
   QString _stringify_game_setting_renumber_error(dovah::notice_code_t code) {
      constexpr char* disambig = "game setting renumber errors";
      //
      switch (code) {
         case dovah::notice_code::form_id_unavailable_for_game_setting:
            return QObject::tr("This desired form ID is unavailable for some reason.", disambig);
         case dovah::notice_code::form_id_is_already_in_use:
            return QObject::tr("The desired form ID is in use by another form.", disambig);
         case dovah::notice_code::form_id_is_reserved_for_other_process:
            return QObject::tr("The desired form ID is currently reserved for use in some other process, such as form creation.", disambig);
         case dovah::notice_code::game_setting_not_in_active_file:
            return QObject::tr("This game setting is not defined in the active file.", disambig);
         case dovah::notice_code::cannot_sever_references_to_target:
            return QObject::tr("One or more forms refer to this game setting's form ID, even though that shouldn't be possible, and DovahKit doesn't know how to edit those forms and thus can't sever those references.", disambig);
         case dovah::notice_code::game_setting_is_not_in_active_file:
            return QObject::tr("You can't renumber a game setting that isn't defined in the active file.", disambig);
      }
      return "";
   }
}
void DovahKitCore::renumber_game_setting(const char* name, QWidget* dialog_parent) {
   if (!this->load_order)
      return;
   dovah::loaded_game_setting setting;
   if (!this->load_order->get_loaded_setting_by_name(name, setting))
      return;
   bool    ok   = false;
   QString text = QInputDialog::getText(dialog_parent, tr("Choose form ID"), tr("What form ID do you want this game setting to use?"), QLineEdit::Normal, QString("%1").arg(setting.formID, 8, 16, QChar('0')).toUpper(), &ok);
   if (!ok)
      return;
   dovah::bare_form_id_t newID = text.toUInt(&ok, 16);
   if (!ok) {
      QMessageBox::critical(
         dialog_parent,
         QObject::tr("Error", "renumber form error"),
         QObject::tr("\"%1\" is not a valid form ID. A form ID is an eight-digit hexadecimal number (that is, each digit is between 0-9 or A-F, inclusive).").arg(text)
      );
      return;
   }
   if (newID == setting.formID) {
      QMessageBox::critical(
         dialog_parent,
         QObject::tr("Error", "renumber form error"),
         QObject::tr("That game setting's form ID already is %1.").arg(QString("%1").arg(newID, 8, 16, QChar('0')).toUpper())
      );
      return;
   }
   //
   if (auto* stub = this->load_order->get_form(dovah::form_type::setting, newID)) {
      auto choice = QMessageBox::question(
         dialog_parent,
         QObject::tr("Warning", "reumber game setting"),
         QObject::tr("Form ID %1 is in use by one or more game settings. Game settings <em>can</em> share the same form ID without causing problems for the game, but community tools like xEdit (a.k.a. TES5Edit and SSEEdit) may not respond properly. Are you sure you wish to use this form ID?", "reumber game setting")
            .arg(QString("%1").arg(newID, 8, 16, QChar('0')).toUpper()),
         QMessageBox::Yes | QMessageBox::No,
         QMessageBox::No
      );
      if (choice != QMessageBox::Yes)
         return;
   }
   //
   bare_form_id_t oldID = setting.formID;
   //
   auto request = this->load_order->request_game_setting_renumber();
   request.setting = name;
   request.set_desired_form_id(newID);
   if (auto code = request.get_notice_code()) {
      _report_game_setting_renumber_error(dialog_parent, _stringify_game_setting_renumber_error(code));
      return;
   }
   request.commit();
   if (auto code = request.get_notice_code()) {
      _report_game_setting_renumber_error(dialog_parent, _stringify_game_setting_renumber_error(code));
      return;
   }
   emit this->gameSettingRenumbered(name, oldID, newID);
}

void DovahKitCore::set_default_object(uint32_t signature, dovah::form_stub* stub) {
   if (!this->load_order)
      return;
   auto* singleton_stub = this->load_order->get_canonical_instance_of_singleton_form(dovah::form_type::default_object_manager, true);
   if (!singleton_stub) {
      //
      // Should be impossible since DOBJ has hardcoded form ID 0x00000031 reserved for it, so 
      // don't even bother handling this.
      //
      #if _DEBUG
         __debugbreak();
      #endif
      return;
   }
   auto loaded = singleton_stub->load().ptr_cast<dovah::loaded_forms::DefaultObjectManager>();
   if (loaded) {
      //
      // This *can* return warning and failure codes, but the UI shouldn't allow the user to 
      // supply any invalid values, so for now, don't bother displaying them.
      //
      auto code = loaded->set_entry(signature, stub);
      switch (code) {
         case dovah::notice_code::default_object_rejected_for_bad_type:
            return;
      }
      //
      // Emit success signal:
      //
      emit this->defaultObjectEntryChanged(signature);
   }
}
void DovahKitCore::set_default_object(uint32_t signature, bare_form_id_t id) {
   if (!this->load_order)
      return;
   auto* stub = this->load_order->get_form(id);
   this->set_default_object(signature, stub);
}

dovah::bsa_archived_file* DovahKitCore::lookup_game_asset(const std::string& path) {
   auto* archives = this->load_order->get_archive_list();
   if (!archives)
      return nullptr;
   return archives->lookup_file(path, true);
}

namespace {
   struct _language_to_encoding {
      const char* language = ""; // must be lowercase
      const char* encoding = "";
   };
   std::array< _language_to_encoding, 19> _language_to_encoding_map = {{
      { "arabic",    "Windows-1256" },
      { "chinese",   "UTF-8" },
      { "czech",     "Windows-1250" },
      { "danish",    "Windows-1252" },
      { "english",   "Windows-1252" },
      { "finnish",   "Windows-1252" },
      { "french",    "Windows-1252" },
      { "german",    "Windows-1252" },
      { "greek",     "Windows-1253" },
      { "hungarian", "Windows-1250" },
      { "italian",   "Windows-1252" },
      { "japanese",  "UTF-8" },
      { "norwegian", "Windows-1252" },
      { "polish",    "Windows-1250" },
      { "portugese", "Windows-1252" },
      { "russian",   "Windows-1251" },
      { "spanish",   "Windows-1252" },
      { "swedish",   "Windows-1252" },
      { "turkish",   "Windows-1254" },
   }};
}
void DovahKitCore::set_encoding(const std::string& name) noexcept {
   auto prior = this->encoding;
   this->encoding = name;
   emit editorEncodingChanged(prior, this->encoding);
}
void DovahKitCore::set_encoding() {
   auto language = dovah::utils::get_user_language_name();
   for (auto& c : language)
      c = tolower(c);
   for (auto& entry : _language_to_encoding_map) {
      if (language == entry.language) {
         this->set_encoding(entry.encoding);
         return;
      }
   }
}

namespace {
   const char* _fallback_encoding_name_for_language(dovah::localization_language l) {
      switch (l) {
         case dovah::localization_language::arabic:     return "Windows-1256";
         case dovah::localization_language::chinese:    return "UTF-8";
         case dovah::localization_language::czech:      return "Windows-1250";
         case dovah::localization_language::danish:     return "Windows-1252";
         case dovah::localization_language::english:    return "Windows-1252";
         case dovah::localization_language::finnish:    return "Windows-1252";
         case dovah::localization_language::french:     return "Windows-1252";
         case dovah::localization_language::german:     return "Windows-1252";
         case dovah::localization_language::greek:      return "Windows-1253";
         case dovah::localization_language::hungarian:  return "Windows-1250";
         case dovah::localization_language::italian:    return "Windows-1252";
         case dovah::localization_language::japanese:   return "UTF-8";
         case dovah::localization_language::norwegian:  return "Windows-1252";
         case dovah::localization_language::polish:     return "Windows-1250";
         case dovah::localization_language::portugese:  return "Windows-1252";
         case dovah::localization_language::russian:    return "Windows-1251";
         case dovah::localization_language::spanish:    return "Windows-1252";
         case dovah::localization_language::swedish:    return "Windows-1252";
         case dovah::localization_language::turkish:    return "Windows-1254";
      }
      return "Windows-1252";
   }
}
QString DovahKitCore::convert_localized_string(const dovah::localized_string& s) const noexcept {
   if (s.localized != dovah::localization_language::none) {
      QTextCodec::ConverterState state;
      auto*   codec = QTextCodec::codecForName("UTF-8");
      QString text  = codec->toUnicode(s.c_str());
      if (state.invalidChars > 0) {
         codec = QTextCodec::codecForName(_fallback_encoding_name_for_language(s.localized));
         text  = codec->toUnicode(s.c_str());
      }
      return text;
   }
   QTextCodec* codec = nullptr;
   if (!this->encoding.empty())
      codec = QTextCodec::codecForName(this->encoding.c_str());
   if (!codec)
      codec = QTextCodec::codecForName("Windows-1252");
   return codec->toUnicode(s.c_str());
}
void DovahKitCore::assign_localized_string(dovah::localized_string& s, const QString& value) const noexcept {
   QTextCodec* codec = nullptr;
   if (!this->encoding.empty())
      codec = QTextCodec::codecForName(this->encoding.c_str());
   if (!codec)
      codec = QTextCodec::codecForName("Windows-1252");
   s.value     = codec->fromUnicode(value);
   s.localized = dovah::localization_language::none;
}

bool DovahKitCore::get_game_path(std::filesystem::path& out, dovah::game game) const noexcept {
   std::wstring value(512, 0);
   const wchar_t* key;
   switch (game) {
      case dovah::game::skyrim_classic:
         key = L"SOFTWARE\\Bethesda Softworks\\Skyrim\\";
         break;
      case dovah::game::skyrim_special:
         key = L"SOFTWARE\\Bethesda Softworks\\Skyrim Special Edition\\";
         break;
      default:
         return false;
   }
   bool success = cobb::windows_registry::get_string_value(cobb::windows_registry::hkey::local_machine, key, L"installed path", value);
   if (success) {
      out = value;
      return true;
   }
   out.clear();
   return false;
}
bool DovahKitCore::get_game_plugins(std::vector<QString>& out, dovah::game game) const noexcept {
   out.clear();
   //
   auto env  = QProcessEnvironment::systemEnvironment();
   QString path;
   switch (game) {
      case dovah::game::skyrim_classic:
         path = env.value("LOCALAPPDATA") + "\\Skyrim\\plugins.txt";
         break;
      case dovah::game::skyrim_special:
         path = env.value("LOCALAPPDATA") + "\\Skyrim Special Edition\\plugins.txt";
         break;
      default:
         return false;
   }
   auto file = QFile(path);
   //
   auto official = list_all_official_plugins(game, true);
   for (auto& s : official)
      out.push_back(s);
   //
   if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      while (!file.atEnd()) {
         auto line = file.readLine();
         if (line[0] == '#')
            continue;
         line = line.trimmed();
         //
         bool found = false;
         for (auto& s : official) {
            if (s.compare(line, Qt::CaseInsensitive) == 0) {
               found = true; // hardcoded file; already in our list; don't allow it to appear twice if plugins.txt wrongly includes it
               break;
            }
         }
         if (found)
            continue;
         //
         out.push_back(line);
      }
   }
   return true;
}

/*static*/ QList<QString> DovahKitCore::list_all_official_plugins(dovah::game game, bool mandatory_only) noexcept {
   QList<QString> out;
   out.push_back("Skyrim.esm"); // game forces this to be 00, and it is not present in plugins.txt
   out.push_back("Update.esm"); // game forces this to be 01, and it is not present in plugins.txt
   if (mandatory_only && game != dovah::game::skyrim_special)
      return out;
   out.push_back("Dawnguard.esm");
   out.push_back("HearthFires.esm");
   out.push_back("Dragonborn.esm");
   //
   // TODO: Creation Club files? We probably shouldn't hardcode those, but rather should have a list 
   // file of them somewhere, so that DovahKit doesn't need to be rebuilt whenever Bethesda adds new 
   // content to the shop.
   //
   return out;
}