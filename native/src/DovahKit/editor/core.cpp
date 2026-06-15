#include "core.h"
#include <fstream>
#include <QDialog>
#include <QFile>
#include <QInputDialog>
#include <QMessageBox>
#include <QProcessEnvironment>
#include <QThread>
#include "helpers/performance.h"
#include "helpers/windows_registry.h"
#include "helpers/qt/strings.h"
#include "dovah/data/game.h"
#include "dovah/form_stub.h"
#include "dovah/form_types.h"
#include "dovah/files/bsa/bsa_load_order.h"
#include "dovah/files/tes_file_writing/results.h"
#include "dovah/files/file_header.h"
#include "dovah/files/bsa/bsa_archived_file.h"
#include "dovah/files/papyrus/compiled_script.h"
#include "dovah/forms/DefaultObjectManager.h"
#include "dovah/utils/form_type_is_cell_child.h"
#include "core_internals/load_task.h"
#include "core_internals/backend_notice_dispatcher.h"
#include "helpers/make_editor_id_for_duplicate.h"
#include "ui/main_window/delete_form_dialog.h"
#include "widgets/widget-models/DKBSACollectionModel.h"
#include <QDebug>
#include "./subsystems/game_inis.h"
#include "./asset_manager/asset_manager.h"
#include "./subsystems/form_info_cache/core.h"
#include "./subsystems/game_localized_strings/core.h"
#include "./subsystems/game_settings/core.h"
#include "./subsystems/message_log/core.h"
#include "./subsystems/options/core.h"
#include "./subsystems/papyrus/core.h"

#include "./form_stub_meta_type.h"
#include "ui/types/quest_alias.h"

#include "dovah/exceptions/invalid_load_order/active_file_is_master_and_there_are_plugins.h"
#include "dovah/exceptions/invalid_load_order/cyclical_dependency_between_files.h"
#include "dovah/exceptions/invalid_load_order/desired_active_file_is_a_dependency.h"
#include "dovah/exceptions/invalid_load_order/load_order_would_have_too_many_files.h"
#include "dovah/exceptions/invalid_load_order/some_files_are_too_new.h"
#include "dovah/exceptions/file_load_failed.h"
#include "dovah/exceptions/form_creation_failed.h"
#include "dovah/exceptions/form_deletion_failed.h"
#include "dovah/exceptions/form_renumber_failed.h"
#include "dovah/exceptions/game_change_failed.h"
#include "dovah/exceptions/game_setting_renumber_failed.h"
#include "dovah/exceptions/invalid_load_order.h"
#include "dovah/notices/base_error.h"
#include "dovah/notices/base_warning.h"
#include "./helpers/backend_error_to_string.h"

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

   void _on_backend_error(const dovah::notices::base_error& notice) {
      DovahKitEditorInternals::backend_notice_dispatcher::get().send(notice);
   }
   void _on_backend_warning(const dovah::notices::base_warning& notice) {
      DovahKitEditorInternals::backend_notice_dispatcher::get().send(notice);
   }
}

namespace {
   namespace _qmetatype_converters {
      QString form_stub(dovah::form_stub* stub) {
         if (!stub)
            return "[NONE:00000000]";
         return QString("[%1:%2]%3")
            .arg(cobb::qt::four_cc_to_string(dovah::form_type_info::lookup(stub->form_type).signature))
            .arg(QString("%1").arg(stub->formID, 8, 16, QChar('0')).toUpper())
            .arg(stub->get_editor_id());
      }
   }
}

DovahKitCore::DovahKitCore() {
   qRegisterMetaType<file_load_stats>(); // needed so that QObject::connect can pass these across threads (by copying them)
   qRegisterMetaType<dovah::form_stub*>();
   qRegisterMetaType<ui::types::quest_alias>();
   QMetaType::registerConverter<dovah::form_stub*, QString>(&_qmetatype_converters::form_stub);
   //
   this->load_order = new dovah::file_load_order;
   this->_configure_load_order();
   //
   {
      using dispatcher_t = DovahKitEditorInternals::backend_notice_dispatcher;
      dispatcher_t& dispatcher = dispatcher_t::get();
      QObject::connect(
         &dispatcher,
         &dispatcher_t::receivedWarning,
         this,
         [this](dovah::notices::base_warning* cloned) {
            emit dovahkit::subsystems::message_log::core::get_or_create().backendWarningReceived(*cloned);
            delete cloned;
         },
         Qt::QueuedConnection
      );
      QObject::connect(
         &dispatcher,
         &dispatcher_t::receivedError,
         this,
         [this](dovah::notices::base_error* cloned) {
            emit dovahkit::subsystems::message_log::core::get_or_create().backendErrorReceived(*cloned);
            delete cloned;
         },
         Qt::QueuedConnection
      );
   }
   //
   {
      //
      // Set up COM on this thread so that it can use DirectXTex.
      //
      HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED); // Qt forcibly uses COINIT_APARTMENTTHREADED via OleInitialize
      if (!FAILED(hr)) {
         this->com_is_ready = true;
      }
      //
      {
         //
         // Prep game INI settings.
         //
         auto bench = cobb::benchmark();
         bench.begin();
         editor::game_inis::get_skyrim();
         editor::game_inis::get_skyrim_prefs();
         bench.end();
         qDebug("Time to generate INIs: %u ms", bench.milliseconds());
      }
   }
   this->bsa_browse_backend = new DKBSACollectionModelBackend(this);
   DKBSACollectionModel::setDefaultBackend(this->bsa_browse_backend);
   //
   // We want to make sure a few systems exist, but we need to construct them AFTER DovahKitCore to avoid 
   // cyclical dependencies / infinite recursion within the constructors (i.e. their constructors access 
   // DovahKitCore, so we need to make sure DovahKitCore is fully constructed by then):
   //
   QTimer::singleShot(0, []() {
      DovahKitAssetManager::get();
      dovahkit::subsystems::form_info_cache::core::get_or_create();
      dovahkit::subsystems::game_localized_strings::core::get_or_create();
      dovahkit::subsystems::game_settings::core::get_or_create();
      dovahkit::subsystems::options::core::get_or_create();
      dovahkit::subsystems::papyrus::core::get_or_create();
   });
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
   //
   if (this->com_is_ready) {
      CoUninitialize(); // every CoInitializeEx call must have a matching CoUninitialize call
      this->com_is_ready = false;
   }
}
void DovahKitCore::_configure_load_order() {
   this->load_order->on_form_create   = &_on_form_created;
   this->load_order->on_form_loss     = &_on_form_loss;
   this->load_order->on_form_renumber = &_on_form_renumber;
   this->load_order->on_mass_renumber = &_on_mass_renumber;
   this->load_order->on_error         = &_on_backend_error;
   this->load_order->on_warning       = &_on_backend_warning;
   this->load_order->adopt_archive_list(*(new dovah::bsa_load_order));
}
void DovahKitCore::abandon_data() {
   emit dataAbandonImminent();
   this->bsa_browse_backend->clear();
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

namespace {
   QString _stringify_load_order_exception(const dovah::exceptions::invalid_load_order& ex) {
      using namespace dovah::exceptions::invalid_load_order_exceptions;
      if (auto* casted = dynamic_cast<const active_file_is_master_and_there_are_plugins*>(&ex)) {
         return QObject::tr(
            "The desired active file is a master, but there are plug-ins in the load order. This "
            "means it's not possible for the active file to be the last file in the load order."
         );
      } else if (auto* casted = dynamic_cast<const cyclical_dependency_between_files*>(&ex)) {
         QString file = QObject::tr("<unknown>");
         if (!casted->seen.empty())
            file = QString::fromStdString(casted->seen.back());

         return QObject::tr(
            "File %1 is the target of a cyclical dependency."
         ).arg(file);
      } else if (auto* casted = dynamic_cast<const desired_active_file_is_a_dependency*>(&ex)) {
         QString active    = QString::fromStdString(casted->active_file);
         QString dependent = QString::fromStdString(casted->dependent_file);

         return QObject::tr(
            "The active file (%1) is listed as another file's (%2) master. This load order is "
            "invalid, because we need the active file at the bottom of the load order."
         ).arg(active).arg(dependent);
      } else if (auto* casted = dynamic_cast<const load_order_would_have_too_many_files*>(&ex)) {
         if (casted->file_counts.active_file_dependencies.has_value()) {
            return QObject::tr(
               "The load order contains too many files (%1). Saving the active file would be "
               "impossible, because a single file can have at most %2 dependencies."
            ).arg(casted->file_counts.active_file_dependencies.value()).arg(254);
         }
         if (casted->file_counts.light == 0) {
            return QObject::tr(
               "The load order contains too many files: %1 heavy. The max is 254 for games "
               "that support light plug-ins, and 255 for older games."
            ).arg(casted->file_counts.heavy);
         }
         return QObject::tr(
            "The load order contains too many files: %1 light and %2 heavy. The cap is 254 "
            "heavy plug-ins and 4096 light plug-ins."
         ).arg(casted->file_counts.light).arg(casted->file_counts.heavy);
      } else if (auto* casted = dynamic_cast<const some_files_are_too_new*>(&ex)) {
         QString list_html = "<ul>";
         for (auto& filename : casted->files) {
            list_html += "<li>" + QString::fromStdString(filename) + "</li>";
         }
         list_html += "</ul>";

         return QObject::tr(
            "<p>Some files in the load order have header version numbers (TES4/HEDR) that are "
            "too new. The game would reject these files.</p>\n\n%1"
         ).arg(list_html);
      }
      return QObject::tr("Unknown problem with the requested load order.");
   }
   QString _stringify_file_load_exception(const dovah::exceptions::file_load_failed& ex) {
      switch (ex.code) {
         case dovah::exceptions::file_load_failed::error_code::no_filename_specified:
            return QObject::tr("No filename specified.", "dovah::exceptions::file_load_failed");
         case dovah::exceptions::file_load_failed::error_code::save_or_load_already_in_progress:
            return QObject::tr("A save or load operation is already in progress.", "dovah::exceptions::file_load_failed");
      }
      if (ex.details.file_load_error) {
         return editor_helpers::backend_error_to_string(*ex.details.file_load_error);
      }
      return QObject::tr("Unknown error.", "dovah::exceptions::file_load_failed");
   }
}
bool DovahKitCore::acquire_load_order_data(bool async) {
   if (this->loading || this->async_loader)
      return false;
   assert(!this->has_data() && "The editor must be made to abandon old data before attempting to acquire new data.");
   this->loading = true;
   if (async) {
      auto* worker = new DovahKitEditorInternals::load_task(*this);
      auto  thread = this->async_loader;
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
      // signal handler, and the assertion message WILL be misleading.
      // 
      // This is probably because emitting signals cross-thread requires thread synchronization and 
      // Qt only knows it's a cross-thread signal by checking the thread affinity of the sender and 
      // the recipient; ergo there must *be* a recipient.
      //
      QObject::connect(worker, &DovahKitEditorInternals::load_task::complete, this, [this, worker](file_load_stats stats) {
         if (auto* bsa_list = this->load_order->get_archive_list())
            this->bsa_browse_backend->setArchives(*bsa_list);
         emit dataAcquireComplete();
         emit fileLoadStatisticsAvailable(stats);
         {
            auto bench = cobb::benchmark();
            bench.begin();
            editor::game_inis::load_inis(this->get_current_game());
            bench.end();
            qDebug("Time to load INIs: %u ms", bench.milliseconds());
         }
      });
      QObject::connect(worker, &DovahKitEditorInternals::load_task::failed, this, [this, worker]() {
         assert(worker->exception);

         QString error_message = tr("An unknown error occurred.");
         try {
            std::rethrow_exception(worker->exception);
         } catch (const dovah::exceptions::invalid_load_order& ex) {
            error_message = _stringify_load_order_exception(ex);
         } catch (const dovah::exceptions::file_load_failed& ex) {
            error_message = _stringify_file_load_exception(ex);
         }
         emit dataAcquireFailed(error_message);
      });
      QObject::connect(worker, &DovahKitEditorInternals::load_task::ended, this, [this, thread]() {
         //
         // We have to call QThread::quit() manually when our work is done. QThreads don't automatically 
         // exit, probably in order to keep spinning an event loop so QObjects living on the thread can 
         // receive signals. See also: <https://stackoverflow.com/a/17094375>.
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
      if (auto* bsa_list = this->load_order->get_archive_list())
         this->bsa_browse_backend->setArchives(*bsa_list);
      emit dataAcquireComplete();
      emit fileLoadStatisticsAvailable(task.stats);
      {
         auto bench = cobb::benchmark();
         bench.begin();
         editor::game_inis::load_inis(this->get_current_game());
         bench.end();
         qDebug("Time to load INIs: %u ms", bench.milliseconds());
      }
   } else {
      emit dataAcquireFailed("Loading failed.");
   }
   return task.result;
}

dovah::game DovahKitCore::get_current_game() const noexcept {
   return this->load_order->get_current_game();
}

float DovahKitCore::assess_load_progress() const noexcept {
   return this->load_order->assess_load_progress();
}

dovah::file_load_order* DovahKitCore::get_file_load_order() noexcept {
   return this->load_order;
}
std::vector<const dovah::tes_file_reading::file_loader*> DovahKitCore::get_loaded_files() const noexcept {
   return this->load_order->get_loaded_files();
}
bool DovahKitCore::loaded_file_is_active(const dovah::tes_file_reading::file_loader& file) const noexcept {
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
void DovahKitCore::save_active_file(std::filesystem::path name_to_use_if_nameless, const dovah::tes_file_writing::write_config& cfg, dovah::tes_file_writing::write_results& results) {
   emit dataSaveImminent();
   try {
      this->load_order->save_active_file(name_to_use_if_nameless, cfg, results);
   } catch (...) {
      emit dataSaveFailed();
      throw; // re-throw
   }
   emit dataSaveComplete();
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

uint32_t DovahKitCore::count_forms_of_type(dovah::form_type ft) const noexcept {
   return this->load_order->count_forms_of_type(ft);
}
dovah::form_stub* DovahKitCore::get_form(bare_form_id_t formID) const noexcept {
   return this->load_order->get_form(formID);
}
dovah::form_stub* DovahKitCore::get_form(dovah::form_type ft, bare_form_id_t formID) const noexcept {
   return this->load_order->get_form(ft, formID);
}
dovah::form_stub* DovahKitCore::get_form_of_probable_type(dovah::form_type ft, bare_form_id_t formID) const noexcept {
   return this->load_order->get_form_of_probable_type(ft, formID);
}
dovah::form_stub* DovahKitCore::get_singleton_form(dovah::form_type ft, bool create_if_missing) const noexcept {
   if (this->load_order)
      return this->load_order->get_canonical_instance_of_singleton_form(ft, create_if_missing);
   return nullptr;
}
bool DovahKitCore::for_each_form(std::function<bool(dovah::form_stub*)> functor) {
   for(auto& info : dovah::form_types)
      if (this->load_order->for_each_form_of_type(info.form_type, functor))
         return true;
   return false;
}
bool DovahKitCore::for_each_form_of_type(dovah::form_type ft, std::function<bool(dovah::form_stub*)> functor) {
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

bool DovahKitCore::get_new_forms_avoid_extended_esl_form_id_range() const noexcept {
   return this->load_order->prefs.new_forms_avoid_extended_esl_form_id_range;
}
void DovahKitCore::set_new_forms_avoid_extended_esl_form_id_range(bool v) noexcept {
   this->load_order->prefs.new_forms_avoid_extended_esl_form_id_range = v;
}

dovah::form_stub* DovahKitCore::create_form_of_type(dovah::form_type ft) {
   if (!this->loaded)
      return nullptr;
   auto* stub = this->load_order->create_form_of_type(ft);
   return stub;
}
dovah::form_creation_request DovahKitCore::request_form_creation(dovah::form_type ft) noexcept {
   return this->load_order->request_form_creation(ft);
}
dovah::form_duplication_request DovahKitCore::request_form_duplication() noexcept {
   return this->load_order->request_form_duplication();
}
dovah::form_renumber_request DovahKitCore::request_form_renumber(dovah::form_stub& stub, bare_form_id_t desiredID) {
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
   QString _stringify_duplicate_form_a_posteriori_error(dovah::exceptions::form_creation_failed::error_code code) {
      using error_code = std::decay_t<decltype(code)>;
      switch (code) {
         case error_code::invalid_form_type:
            return QObject::tr("An internal program error occurred: DovahKit tried to create a form but supplied a bad form type.");
         case error_code::no_active_file:
            return QObject::tr("There is neither an active file in the load order nor any room in the load order for a new file.");
         case error_code::no_form_id_available:
            return QObject::tr("Not enough form IDs are left in the active file to use for the duplicated form(s). (This error was detected late; please report this to DovahKit's developer as a bug.)");
         case error_code::unimplemented_form_type:
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
         case error_code::cannot_sever_references_to_none_stub:
            return QObject::tr("DovahKit needed to select a form ID to use for the new form. The chosen form ID is the target of one or more dangling references, and DovahKit does not know how to sever those references, so the form creation process could not continue.");
      }
      return "";
   }
}
dovah::form_stub* DovahKitCore::duplicate_form(dovah::form_stub& original, QWidget* dialog_parent) {
   auto  request = this->request_form_duplication();
   request.set_target(&original);
   //
   if (!dovah::form_type_is_cell_child(original.form_type)) { // shouldn't ever happen for the Object Window, but eh
      QString suggestion = editor_helpers::make_editor_id_for_duplicate(original.get_editor_id());
      bool    ok         = false;
      QString editor_id  = QInputDialog::getText(dialog_parent, tr("Set editor ID"), tr("Editor ID:"), QLineEdit::Normal, suggestion, &ok);
      if (!ok)
         return nullptr;
      request.editorID = editor_id.toStdString();
   }

   using exception  = dovah::exceptions::form_creation_failed;
   using error_code = exception::error_code;

   dovah::form_stub* root_created_form = nullptr;
   try {
      root_created_form = request.commit();
   } catch (const dovah::exceptions::form_creation_failed& ex) {
      QString error_text;
      if (ex.code == error_code::no_form_id_available) {
         size_t needed_forms    = ex.details.form_ids_needed;
         size_t no_reservations = ex.details.form_ids_missing;
         size_t other_failures  = ex.details.failure_count - no_reservations;
         
         if (ex.details.failure_count > 1) {
            if (no_reservations) {
               error_text = tr("A total of %1 forms need to be created, but the file only has %2 free form IDs to spare.").arg(needed_forms).arg(no_reservations);
               if (other_failures)
                  error_text += tr(" Additional errors were encountered while validating %3 other forms.").arg(other_failures);
               error_text += tr(" No forms were created.");
            } else {
               error_text = tr("A total of %1 forms need to be created, but errors were encountered while validating %2 of those forms.").arg(needed_forms).arg(other_failures);
               error_text += tr(" No forms were created.");
            }
         } else {
            if (no_reservations) {
               error_text = tr("The file doesn't have any free form IDs that a new form could use.");
            } else {
               error_text = tr("An error occurred.");
            }
         }
      } else {
         error_text = _stringify_duplicate_form_a_posteriori_error(ex.code);
      }
      _report_duplicate_form_error(dialog_parent, error_text);
   }
   return root_created_form;
}

void DovahKitCore::delete_form(dovah::form_stub& target, QWidget* dialog_parent) {
   using exception  = dovah::exceptions::form_deletion_failed;
   using error_code = exception::error_code;

   this->delete_form(target,
      //
      // After-gather callback:
      //
      [this, dialog_parent](const dovah::form_deletion_request& request) {
         if (dialog_parent) {
            //
            // Show a confirmation prompt.
            //
            auto* confirm = new DeleteFormDialog(dialog_parent);
            confirm->updateFromDeletionRequest(request);
            auto  result = confirm->exec();
            delete confirm;
            if (result == QDialog::Rejected)
               return false;
         }
         return true;
      },
      [this, dialog_parent](const dovah::exceptions::form_deletion_failed& ex) {
         QString text;
         switch (ex.code) {
            case error_code::no_active_file:
               text = tr("There is neither an active file nor room in the load order for an active file.");
               break;
            case error_code::form_is_hardcoded:
               text = tr("The form is hardcoded into the game engine and cannot be deleted.");
               break;
            case error_code::unimplemented_form_type:
               text = tr("DovahKit doesn't currently support this form type, which means that it cannot flag the form as deleted.");
               break;
            case error_code::cannot_load_all_users_of_this_form:
               text = tr("One of the forms that uses this form is of an unsupported type, which means that that use cannot be severed.");
               break;
         }
         QMessageBox::critical(
            dialog_parent,
            QObject::tr("Error", "delete form error"),
            QObject::tr("Unable to delete this form. %1").arg(text)
         );
      },
      //
      // After-complete callback:
      //
      [](const dovah::form_deletion_request& request) {}
   );
}
void DovahKitCore::delete_form(
   dovah::form_stub& target,
   std::function<bool(const dovah::form_deletion_request&)> after_gather,
   std::function<void(const dovah::exceptions::form_deletion_failed&)> on_gather_error,
   std::function<void(const dovah::form_deletion_request&)> after_complete
) {
   try {
      auto request = this->load_order->request_form_deletion(target);
      if (!after_gather(request))
         return;
      
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
      
      request.commit();

      for (auto& entry : formIDs)
         emit this->formDeletionComplete(entry.id, entry.flagged);
      
      after_complete(request);
   } catch (const dovah::exceptions::form_deletion_failed& ex) {
      on_gather_error(ex);
   }
}

namespace {
   template<
      typename DecideToMoveFunctor,
      typename VerifyDestinationIDFunctor,
      typename FailureHandlerFunctor
   >
      requires requires(
         DecideToMoveFunctor&& decide,
         VerifyDestinationIDFunctor&& verify,
         FailureHandlerFunctor&& on_fail,
         const dovah::form_stub& stub,
         uint32_t form_id,
         const dovah::exceptions::form_renumber_failed& exception
      ) {
         { decide(stub) } -> std::same_as<bool>;
         { verify(stub, form_id) } -> std::same_as<bool>;
         { on_fail(exception) };
      }
   bool _bulk_renumber_active_file_forms(
      dovah::file_load_order& lo,
      uint32_t min_form_id,
      DecideToMoveFunctor&& decide_to_move_functor,
      VerifyDestinationIDFunctor&& verify_dst_id_functor,
      FailureHandlerFunctor&& failure_handler_functor
   ) {
      std::vector<std::pair<uint32_t, dovah::form_stub*>> pending;
      bool insufficient_ids = false;
      lo.for_each_active_file_form(
         [
            &lo,
            min_form_id,
            &decide_to_move_functor,
            &verify_dst_id_functor,
            &insufficient_ids,
            &pending
         ](dovah::form_stub* stub) {
            if (stub->is_hardcoded() || stub->is_injected())
               return false;
            if (!lo.is_defined_in_active_file(*stub))
               return false;

            if (!decide_to_move_functor(*stub))
               return false;

            uint32_t dst_id = [&]() {
               uint32_t search_from = min_form_id;
               if (!pending.empty())
                  search_from = pending.back().first + 1;
               return lo.find_first_free_form_id_in_active_file(search_from);
            }();
            if (dst_id == 0 || !verify_dst_id_functor(*stub, dst_id)) {
               insufficient_ids = true;
               return true;
            }
            pending.push_back(std::pair{ dst_id, stub });
            return false;
         }
      );
      if (insufficient_ids) {
         return false;
      }

      try {
         for (auto& pair : pending) {
            auto request = lo.request_form_renumber(*pair.second, pair.first);
            request.commit();
         }
      } catch (const dovah::exceptions::form_renumber_failed& ex) {
         failure_handler_functor(ex);
         return false;
      }
      return true;
   }
}
bool DovahKitCore::try_compact_form_ids(bool only_move_if_out_of_range, bool allow_bees, bool require_in_esl_range) {
   bool active_is_light = false;
   if (auto* file = this->get_active_file_header())
      active_is_light = file->is_light();

   uint32_t form_id_to_record_id_mask = active_is_light ? 0x00000FFF : 0x00FFFFFF;

   return _bulk_renumber_active_file_forms(
      *this->load_order,
      allow_bees ? 0 : dovah::max_hardcoded_form_id + 1,

      // decide whether to move:
      [only_move_if_out_of_range, allow_bees, form_id_to_record_id_mask](const dovah::form_stub& stub) {
         if (only_move_if_out_of_range) {
            uint32_t masked = stub.formID & form_id_to_record_id_mask;
            if (masked <= 0xFFF)
               if (allow_bees || masked > dovah::max_hardcoded_form_id)
                  return false;
         }
         return true;
      },

      // verify chosen form ID:
      [allow_bees, require_in_esl_range, active_is_light, form_id_to_record_id_mask](const dovah::form_stub& stub, uint32_t dst_id) {
         if (require_in_esl_range) {
            if (active_is_light) {
               //
               // The backend will not have chosen a form ID that is wholly outside the 
               // range of form IDs available in the active file [as it currently exists].
               //
            } else {
               if ((dst_id & 0x00FFFFFF) > 0xFFF)
                  return false;
            }
         }
         if (!allow_bees && (dst_id & form_id_to_record_id_mask) <= dovah::max_hardcoded_form_id)
            return false;
         return true;
      },

      // handle failure:
      [](const dovah::exceptions::form_renumber_failed& ex) {
         //
         // Recent code changes *should* make this impossible; we're not injecting forms, 
         // and none-stubs' form IDs should no longer be treated as available if those 
         // none-stubs can't be deleted. Still, I'm keeping this catch-block here until 
         // I refactor and clean up the broader file_load_order internals during sustain.
         //
         // As of this writing, there are no other failure cases for non-hardcoded forms.
         //
      }
   );
}
bool DovahKitCore::try_move_form_ids_out_of_hardcoded_ambiguous_range() {
   bool active_is_light = false;
   if (auto* file = this->get_active_file_header())
      active_is_light = file->is_light();

   uint32_t form_id_to_record_id_mask = active_is_light ? 0x00000FFF : 0x00FFFFFF;

   return _bulk_renumber_active_file_forms(
      *this->load_order,
      dovah::max_hardcoded_form_id + 1,

      // decide whether to move:
      [form_id_to_record_id_mask](const dovah::form_stub& stub) {
         return (stub.formID & form_id_to_record_id_mask) <= dovah::max_hardcoded_form_id;
      },

      // verify chosen form ID:
      [form_id_to_record_id_mask](const dovah::form_stub& stub, uint32_t dst_id) {
         uint32_t masked = stub.formID & form_id_to_record_id_mask;
         return dst_id > dovah::max_hardcoded_form_id;
      },

      // handle failure:
      [](const dovah::exceptions::form_renumber_failed& ex) {
         //
         // Recent code changes *should* make this impossible; we're not injecting forms, 
         // and none-stubs' form IDs should no longer be treated as available if those 
         // none-stubs can't be deleted. Still, I'm keeping this catch-block here until 
         // I refactor and clean up the broader file_load_order internals during sustain.
         //
         // As of this writing, there are no other failure cases for non-hardcoded forms.
         //
      }
   );
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
void DovahKitCore::edit_game_setting(const char* name, const dovah::game_setting_value& value) {
   if (!this->load_order)
      return;
   auto request = this->load_order->request_game_setting_change();
   request.setting.name  = name;
   request.setting.value = value;
   request.commit();
   emit this->gameSettingValueChanged(name);
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
   QString _stringify_game_setting_renumber_error(const dovah::exceptions::game_setting_renumber_failed& ex) {
      using error_code = std::decay_t<decltype(ex)>::error_code;

      const char* disambig = "game setting renumber errors";
      //
      switch (ex.code) {
         case error_code::form_id_is_zero:
            return QObject::tr("Zero is not a valid form ID.", disambig);
         case error_code::form_id_is_in_hardcoded_range:
            return QObject::tr("The desired form ID is in the hardcoded range.", disambig);
         case error_code::form_id_is_out_of_bounds:
            return QObject::tr("The desired form ID is out of bounds.", disambig);
         case error_code::form_id_is_occupied:
            return QObject::tr("The desired form ID is in use by another form (not another game setting).", disambig);
         case error_code::form_id_is_reserved:
            return QObject::tr("The desired form ID is currently reserved for use in some other process, such as form creation.", disambig);
         case error_code::no_active_file:
            return QObject::tr("There is no active file, nor room in the load order for a new file.", disambig);
         case error_code::cannot_sever_references_to_setting:
            return QObject::tr("One or more forms refer to this game setting's form ID, even though that shouldn't be possible, and DovahKit doesn't know how to edit those forms and thus can't sever those references.", disambig);
         case error_code::setting_is_not_in_active_file:
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
   try {
      auto request = this->load_order->request_game_setting_renumber();
      request.setting = name;
      request.set_desired_form_id(newID);
      request.commit();
   } catch (const dovah::exceptions::game_setting_renumber_failed& ex) {
      _report_game_setting_renumber_error(dialog_parent, _stringify_game_setting_renumber_error(ex));
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
      loaded->set_entry(signature, stub);
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

const dovah::bsa_load_order* DovahKitCore::get_bsa_load_order() {
   return this->load_order->get_archive_list();
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