#include "core.h"
#include <QFile>
#include <QInputDialog>
#include <QMessageBox>
#include <QProcessEnvironment>
#include <QThread>
#include "../helpers/performance.h"
#include "../helpers/windows_registry.h"
#include "../dovah/form_stub.h"
#include "../dovah/files/tes_file_reading/file.h"
#include "core_internals/load_task.h"
#include "helpers/make_editor_id_for_duplicate.h"
#include <QDebug>

namespace {
   void _on_form_created(dovah::form_stub* stub) {
      if (stub)
         emit DovahKitCore::get().formCreated(stub);
   }
}
DovahKitCore::DovahKitCore() {
   qRegisterMetaType<file_load_stats>(); // needed so that QObject::connect can pass these across threads (by copying them)
   //
   this->load_order = new dovah::file_load_order;
   this->_configure_load_order();
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
   this->load_order->on_form_create = &_on_form_created;
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

float DovahKitCore::assess_load_progress() const noexcept {
   return this->load_order->assess_load_progress();
}
const dovah::file_read_error& DovahKitCore::get_last_read_error() const noexcept {
   return this->load_order->load_error;
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
   return this->load_order->index_of_active_file() != dovah::file_load_order::invalid_load_prefix;
}
bool DovahKitCore::save_active_file(std::filesystem::path name_to_use_if_nameless, const dovah::tes_file_writing::write_config* cfg) {
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
int DovahKitCore::load_order_index_of_file(const std::filesystem::path& filename) {
   return this->load_order->index_of_loaded_file(filename.string());
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
bool DovahKitCore::form_is_from_active_file(const dovah::form_stub* stub) const noexcept {
   if (!this->loaded)
      return false;
   return this->load_order->form_is_from_active_file(stub);
}
bool DovahKitCore::form_is_from_active_file(bare_form_id_t formID) const noexcept {
   if (!this->loaded)
      return false;
   return this->load_order->form_is_from_active_file(formID);
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

namespace {
   void _report_duplicate_form_error(QWidget* parent, const QString& text) {
      QMessageBox::critical(
         parent,
         QObject::tr("Error", "create new form error"),
         QObject::tr("Unable to duplicate this form. %1").arg(text)
      );
   }
   QString _stringify_duplicate_form_a_posteriori_error(dovah::form_duplication_request::error_code code) {
      using error_code = dovah::form_duplication_request::error_code;
      switch (code) {
         case error_code::bad_form_type_requested:
            return QObject::tr("An internal program error occurred: DovahKit tried to create a form but supplied a bad form type.");
         case error_code::no_active_file:
            //
            // This should've been caught beforehand.
            //
            break;
         case error_code::no_form_id_available:
            //
            // This should've been caught beforehand.
            //
            break;
         case error_code::unsupported_form_type_requested:
            return QObject::tr("DovahKit does not support editing this form type.");
         case error_code::invalid_parent_child_relationship:
            return QObject::tr("The specified parent form cannot have a child form of this type.");
         case error_code::exterior_grid_coordinates_already_taken:
            return QObject::tr("The specified worldspace already has an exterior cell at the desired grid coordinates.");
         case error_code::cannot_create_reference_with_no_parent_cell:
            return QObject::tr("References cannot be created outside of a cell.");
         case error_code::interior_cell_clone_cannot_have_parent:
            return QObject::tr("Interior cells cannot have a parent worldspace.");
         case error_code::exterior_cell_clone_must_have_parent:
            return QObject::tr("Exterior cells must have a parent worldspace.");
      }
      return "";
   }
}
dovah::form_stub* DovahKitCore::duplicate_form(dovah::form_stub& original, QWidget* dialog_parent) {
   using error_code = dovah::form_duplication_request::error_code;
   //
   auto  request = this->request_form_duplication();
   request.set_target(&original);
   if (request.has_error()) {
      auto       errors = request.get_error_codes();
      error_code error  = error_code::none;
      //
      unsigned int needed_forms    = request.get_total_form_count();
      unsigned int no_reservations = 0;
      unsigned int other_failures  = 0;
      //
      for (auto e : errors) {
         if (e == error_code::none)
            continue;
         if (e == error_code::no_active_file) {
            _report_duplicate_form_error(dialog_parent, tr("There is neither an active file in the load order nor any room in the load order for a new file."));
            return nullptr;
         }
         if (e == error_code::no_form_id_available) {
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
   if (main_error != error_code::none) {
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
   //
   // TODO: Confirmation message
   //
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

bool DovahKitCore::get_game_path(std::filesystem::path& out) const noexcept {
   std::wstring value(512, 0);
   bool success = cobb::windows_registry::get_string_value(cobb::windows_registry::hkey::local_machine, L"SOFTWARE\\Bethesda Softworks\\Skyrim\\", L"installed path", value);
   if (success) {
      out = value;
      return true;
   }
   out.clear();
   return false;
}
bool DovahKitCore::get_game_plugins(std::vector<QString>& out) const noexcept {
   out.clear();
   //
   auto env  = QProcessEnvironment::systemEnvironment();
   auto file = QFile(env.value("LOCALAPPDATA") + "\\Skyrim\\plugins.txt");
   if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
      return false;
   //
   out.push_back("Skyrim.esm"); // game forces this to be 00, and it is not present in plugins.txt
   out.push_back("Update.esm"); // game forces this to be 01, and it is not present in plugins.txt
   //
   // TODO: Apparently Skyrim Special also hardcodes the DLCs and omits them from plugins.txt. 
   // If we want the SSE plugins.txt (we need an argument for that), then we should pull from 
   // the right game's directory and for SSE, hardcode the DLCs both at this spot and in the 
   // loop below.
   //
   while (!file.atEnd()) {
      auto line = file.readLine();
      if (line[0] == '#')
         continue;
      line = line.trimmed();
      if (line.compare("Skyrim.esm", Qt::CaseInsensitive) == 0) // hardcoded file; already in our list; don't allow it to appear twice if plugins.txt wrongly includes it
         continue;
      if (line.compare("Update.esm", Qt::CaseInsensitive) == 0) // hardcoded file; already in our list; don't allow it to appear twice if plugins.txt wrongly includes it
         continue;
      out.push_back(line);
   }
   return true;
}