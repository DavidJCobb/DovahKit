#include "./CameraPathFormsModel.h"
#include <QColor>
#include <QMessageBox>
#pragma region Drag and drop
   #include <QByteArray>
   #include <QDataStream>
   #include <QMimeData>
#pragma endregion
#include "dovah/datastores/camera_paths/node.h"
#include "dovah/forms/CameraPath.h"
#include "dovah/form_stub.h"
#include "dovahscript/dovahscript_host.h"
#include "editor/core.h"
#include "editor/form_stub_meta_type.h"
#include "editor/helpers/make_editor_id_for_duplicate.h"
#include "editor/subsystems/message_log/core.h"
#include "dovah/files/tes_file_reading/file_loader.h"

// for pushing warnings to the log window:
#include "dovah/datastores/camera_paths/warnings/cyclical_parent_relationships.h"
#include "dovah/datastores/camera_paths/warnings/cyclical_sibling_relationships.h"
#include "dovah/datastores/camera_paths/warnings/form_has_multiple_next_siblings.h"
#include "dovah/datastores/camera_paths/warnings/inconsistent_parentage.h"
#include "dovah/datastores/camera_paths/warnings/previous_sibling_is_not_as_expected.h"
#include "dovah/datastores/camera_paths/warnings/siblings_have_mismatched_parents.h"
#include "dovah/datastores/camera_paths/warnings/sibling_is_an_ancestor.h"
#include "editor/helpers/form_identifiers_to_string.h"

namespace {
   constexpr const char* const mime_type = "application/dovah-kit.camera-path-forms-model.node";
}
namespace datastore_warnings {
   using namespace dovah::datastores::impl::camera_paths::warnings;
}

CameraPathFormsModel::CameraPathFormsModel(QObject* parent) : QAbstractItemModel(parent) {
   #pragma region Set up datastore handlers
      this->_datastore.handlers.delete_camera_path = [this](dovah::form_stub& stub) {
         DovahKitCore::get().delete_form(
            stub,
            [](const dovah::form_deletion_request&) {
               return true;
            },
            [this](const dovah::exceptions::form_deletion_failed& ex) {
               this->_handler_state.any_deletions_failed = true;
            },
            [](const dovah::form_deletion_request& request) {}
         );
      };
   #pragma endregion
   #pragma region Set up datastore callbacks
      {
         auto& cb_set = this->_datastore.callbacks.reset;
         cb_set.before = [this]() { this->beginResetModel(); };
         cb_set.after = [this]() { this->endResetModel(); };
      }
      {
         auto& cb_set = this->_datastore.callbacks.form_data_modified;
         cb_set.before = [this](dovah::form_stub& stub) {
            DovahKitCore::get().formModificationImminent(&stub);
         };
         cb_set.after = [this](dovah::form_stub& stub) {
            DovahKitCore::get().formModified(&stub);
         };
      }
      {
         auto& cb_set = this->_datastore.callbacks.node_deleted;
         cb_set.before = [this](const datastore_node& node) {
            if (node.parent) {
               auto qmi = _qmi_for_item(*node.parent);
               size_t i = node.parent->index_of_child(node);
               this->beginRemoveRows(qmi, i, i);
               this->_callback_state.emitted_last_deletion = true;
            }
         };
         cb_set.after = [this](const uint32_t form_id) {
            if (this->_callback_state.emitted_last_deletion)
               this->endRemoveRows();
            this->_callback_state.emitted_last_deletion = false;
         };
      }
      {
         auto& cb_set = this->_datastore.callbacks.node_placed;
         cb_set.before = [this](const datastore_node& subject, const datastore_item& parent_after, size_t index) {
            if (subject.parent) {
               auto&  parent_prior = *subject.parent;
               size_t from = parent_prior.index_of_child(subject);
               auto to        = index;
               auto qmi_prior = _qmi_for_item(parent_prior);
               auto qmi_after = _qmi_for_item(parent_after);
               if (to >= from)
                  ++to;
               this->_callback_state.last_node_placement_was_an_insertion = false;
               this->beginMoveRows(qmi_prior, from, from, qmi_after, to);
            } else {
               this->_callback_state.last_node_placement_was_an_insertion = true;
               this->beginInsertRows(_qmi_for_item(parent_after), index, index);
            }
         };
         cb_set.after = [this](const datastore_node& node) {
            this->_recache(node);
            if (this->_callback_state.last_node_placement_was_an_insertion)
               this->endInsertRows();
            else
               this->endMoveRows();
         };
      }
   #pragma endregion

   auto& scripthost = DovahscriptHost::get();
   QObject::connect(&scripthost, &DovahscriptHost::scriptEnded, this, [this]() {
      //
      // Dovahscript APIs could allow a script to modify an idle's parent and/or previous sibling 
      // such that its position in the hierarchy is invalid. Error-checking those after the tree 
      // is already built is bloody difficult, so the datastore currently doesn't implement that. 
      // The only recourse we have, for any situation where an idle's graph, parent, or previous 
      // sibling might be changed out from udner the datastore, is to rebuild the datastore from 
      // scratch.
      //
      this->_rebuild_datastore();
   });

   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent,  this, &CameraPathFormsModel::_on_game_data_abandon);
   QObject::connect(&editor, &DovahKitCore::dataAcquireComplete,  this, &CameraPathFormsModel::_on_game_data_acquired);
   QObject::connect(&editor, &DovahKitCore::formCreated,          this, [this](dovah::form_stub* stub) { this->_on_form_created(*stub); });
   QObject::connect(&editor, &DovahKitCore::formModified,         this, [this](dovah::form_stub* stub) { this->_on_form_modified(*stub); });
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub, bool flag) { this->_on_form_deletion_imminent(*stub, flag); });
   QObject::connect(&editor, &DovahKitCore::formDeletionComplete, this, [this](dovah::bare_form_id_t id, bool flag) { this->_on_form_deleted(id, flag); });
   if (editor.has_data()) {
      this->_rebuild_datastore();
   }
}
#pragma region Node/QMI utils and node lookups
   QModelIndex CameraPathFormsModel::_qmi_for_model_root() const {
      return {};
   }
   QModelIndex CameraPathFormsModel::_qmi_for_item(const datastore_item& item) const {
      if (&item == &this->_datastore.root)
         return _qmi_for_model_root();
      auto* node = dynamic_cast<const datastore_node*>(&item);
      assert(node != nullptr && "Don't pass a bad pointer OR the root of a datastore other than our own!");
      return _qmi_for_node(*node);
   }
   QModelIndex CameraPathFormsModel::_qmi_for_node(const datastore_node& subject, int column) const {
      if (!subject.parent)
         return {};
      size_t i = subject.parent->index_of_child(subject);
      assert(i != datastore_item::index_of_none);
      return this->createIndex(i, column, (quintptr)(const datastore_item*)subject.parent);
   }
   QModelIndex CameraPathFormsModel::_qmi_for_child_node(int row, int column, const datastore_item& parent) const {
      return this->createIndex(row, column, (quintptr)&parent);
   }
   bool CameraPathFormsModel::_qmi_is_child_of(const QModelIndex& qmi, const datastore_item& node) const {
      return qmi.internalPointer() == &node;
   }

   const CameraPathFormsModel::datastore_node* CameraPathFormsModel::_child_node_by_row(const datastore_item& parent, size_t row) const {
      auto& list = parent.children;
      auto  size = list.size();
      if (row >= size)
         return nullptr;
      return parent.children[row];
   }

   const CameraPathFormsModel::datastore_item* CameraPathFormsModel::_item_for_qmi(const QModelIndex& qmi) const {
      if (!qmi.isValid())
         return &this->_datastore.root;
      return _node_for_qmi(qmi);
   }
   CameraPathFormsModel::datastore_item* CameraPathFormsModel::_item_for_qmi(const QModelIndex& qmi) {
      return const_cast<datastore_item*>(std::as_const(*this)._item_for_qmi(qmi));
   }
   const CameraPathFormsModel::datastore_node* CameraPathFormsModel::_node_for_qmi(const QModelIndex& qmi) const {
      if (!qmi.isValid())
         return nullptr;
      auto* parent = (datastore_item*)qmi.internalPointer();
      if (!parent)
         return nullptr;
      return _child_node_by_row(*parent, qmi.row());
   }
   CameraPathFormsModel::datastore_node* CameraPathFormsModel::_node_for_qmi(const QModelIndex& qmi) {
      return const_cast<datastore_node*>(std::as_const(*this)._node_for_qmi(qmi));
   }

   const CameraPathFormsModel::datastore_node* CameraPathFormsModel::_child_node_for_qmi(const QModelIndex& parent_qmi, int row) const {
      if (parent_qmi == _qmi_for_model_root()) {
         return _child_node_by_row(this->_datastore.root, row);
      }
      const auto* parent_node = _node_for_qmi(parent_qmi);
      if (!parent_node)
         return nullptr;
      return _child_node_by_row(*parent_node, row);
   }
   CameraPathFormsModel::datastore_node* CameraPathFormsModel::_child_node_for_qmi(const QModelIndex& parent_qmi, int row) {
      return const_cast<datastore_node*>(std::as_const(*this)._child_node_for_qmi(parent_qmi, row));
   }
#pragma endregion

#pragma region Form events
   void CameraPathFormsModel::_on_game_data_acquired() {
      this->_rebuild_datastore();
   }
   void CameraPathFormsModel::_on_game_data_abandon() {
      this->beginResetModel();
      this->_cache.clear();
      this->_drag_and_drop.clear();
      this->_datastore.reset();
      this->endResetModel();
   }
   void CameraPathFormsModel::_on_form_created(dovah::form_stub& stub) {
      if (stub.form_type != datastore_type::relevant_form_type)
         return;
      if (this->_callback_state.ignore_next_created_camera_path) {
         this->_callback_state.ignore_next_created_camera_path = false;
         return;
      }
      this->_datastore.on_form_created(stub);
   }
   void CameraPathFormsModel::_on_form_modified(dovah::form_stub& stub) {
      if (stub.form_type == datastore_type::relevant_form_type)
         this->_recache(stub);
   }
   void CameraPathFormsModel::_on_form_deletion_imminent(dovah::form_stub& stub, bool just_being_flagged) {
      if (!just_being_flagged) {
         if (auto* node = this->_datastore.node_by_stub(stub)) {
            this->_drag_and_drop.untrack(*node);
         }
         this->_datastore.on_before_form_fully_deleted(stub);
         return;
      }
      if (stub.form_type == datastore_type::relevant_form_type) {
         this->_recache(stub);
      }
   }
   void CameraPathFormsModel::_on_form_deleted(uint32_t form_id, bool just_being_flagged) {
      if (!just_being_flagged)
         return;
      auto& editor = DovahKitCore::get();
      auto* stub   = editor.get_form(form_id);
      if (!stub)
         return;
      if (stub->form_type == datastore_type::relevant_form_type) {
         this->_recache(*stub);
      }
   }
#pragma endregion

void CameraPathFormsModel::_rebuild_datastore() {
   this->_cache.clear();
   this->_drag_and_drop.clear();

   auto* flo = DovahKitCore::get().get_file_load_order();
   if (!flo) {
      this->beginResetModel();
      this->_datastore.reset();
      this->endResetModel();
      return;
   }

   this->beginResetModel();
   this->_datastore.build(*flo);
   {
      auto _recache_tree = [this](this auto&& recurse, datastore_node& subject) -> void {
         this->_recache(subject);
         for (datastore_node* child : subject.children) {
            recurse(*child);
         }
      };
      for (datastore_node* node : this->_datastore.root.children) {
         _recache_tree(*node);
      }
   }
   {
      auto& list = this->_datastore.warnings;
      if (!list.empty()) {
         auto& logger = dovahkit::subsystems::message_log::core::get();
         for (const auto* warning : list) {
            QString text;
            if (auto* casted = dynamic_cast<const datastore_warnings::cyclical_parent_relationships*>(warning)) {
               text = tr("Form %1's parent chain forms a cyclical reference.")
                  .arg(editor_helpers::form_identifiers_to_string(&casted->subject.stub));
            } else if (auto* casted = dynamic_cast<const datastore_warnings::cyclical_sibling_relationships*>(warning)) {
               text = tr("Form %1's previous-sibling chain forms a cyclical reference.")
                  .arg(editor_helpers::form_identifiers_to_string(&casted->subject.stub));
            } else if (auto* casted = dynamic_cast<const datastore_warnings::form_has_multiple_next_siblings*>(warning)) {
               text = tr(
                  "Multiple camera paths are fighting to have %1 as their previous sibling. This can "
                  "happen if the camera path tree has been overridden improperly by a mod."
               )
                  .arg(editor_helpers::form_identifiers_to_string(&casted->subject.stub));
            } else if (auto* casted = dynamic_cast<const datastore_warnings::inconsistent_parentage*>(warning)) {
               text = tr("Somehow, camera path %1 is not present in its parent's child list.")
                  .arg(editor_helpers::form_identifiers_to_string(&casted->subject.stub));
            } else if (auto* casted = dynamic_cast<const datastore_warnings::previous_sibling_is_not_as_expected*>(warning)) {
               if (auto* node = casted->sibling.intended) {
                  text = tr("After the camera path tree was fully built, %1 expected to be the next sibling of %2.");
                  text = text.arg(editor_helpers::form_identifiers_to_string(&casted->subject.stub));
                  text = text.arg(editor_helpers::form_identifiers_to_string(&node->stub));
               } else {
                  text = tr("After the camera path tree was fully built, %1 expected to be the first (or possibly only) child of its parent.");
                  text = text.arg(editor_helpers::form_identifiers_to_string(&casted->subject.stub));
               }

               QString instead;
               if (auto* node = casted->sibling.actual) {
                  instead = tr("Instead, the camera path is located after %1.");
                  instead = instead.arg(editor_helpers::form_identifiers_to_string(&node->stub));
               } else {
                  instead = tr("Instead, the camera path is its parent's first or only child.");
               }

               text = tr("%1 %2", "datastore warning sentence order for 'previous sibling is not as expected'").arg(text).arg(instead);
            } else if (auto* casted = dynamic_cast<const datastore_warnings::sibling_is_an_ancestor*>(warning)) {
               text = tr("Form %1 has a previous sibling that is one of its ancestor idles.")
                  .arg(editor_helpers::form_identifiers_to_string(&casted->subject.stub));
            } else if (auto* casted = dynamic_cast<const datastore_warnings::siblings_have_mismatched_parents*>(warning)) {
               text = tr("Form %1 has a different parent from one of its previous siblings.")
                  .arg(editor_helpers::form_identifiers_to_string(&casted->subject.stub));
            } else {
               text = tr(
                  "An unknown warning occurred when loading the camera path trees. Please contact DovahKit's developer "
                  "so that a warning message can be added for this. If possible, please make backup copies of the file(s) "
                  "that you had loaded at the time, and be ready to send those along, so the developer can reproduce the "
                  "problem on their end."
               );
            }
            logger.addLogItem({
               text,
               ui::types::log_item_type::warning,
               ui::types::log_item_context::unspecified
            });
         }
      }
   }
   this->endResetModel();
}
void CameraPathFormsModel::_recache(const datastore_node& node) {
   auto& stub  = node.stub;
   auto& cache = this->_cache[(datastore_node*)&node];
   cache.display_string = QString::fromStdString(stub.get_editor_id());
   if (stub.is_edited()) {
      cache.display_string += tr(" *", "form indicator: edited");
   }
   if (stub.is_deleted()) {
      cache.display_string += tr(" (D)", "form indicator: deletion");
   }
}
void CameraPathFormsModel::_recache(const dovah::form_stub& stub) {
   auto* node = this->_datastore.node_by_stub(stub);
   if (node) {
      this->_recache(*node);
      auto tl = _qmi_for_node(*node);
      auto br = _qmi_for_node(*node, this->columnCount({}) - 1);
      emit dataChanged(tl, br);
   }
}

#pragma region Form utils
   dovah::form_stub* CameraPathFormsModel::_try_silently_create_camera_path(QString editor_id) {
      //
      // Normally, we automatically react to the creation of an IDLE form occurring 
      // anywhere in DovahKit: we tell the datastore that a new IDLE has been created, 
      // and in turn, the datastore's own callbacks lead back to us invoking callbacks 
      // on QAbstractItemModel for when a row is inserted. This is sufficient for when 
      // IDLEs are created outside of our control.
      // 
      // The IDLE is created "bare," with no parent and no behavior graph, so the row 
      // would be inserted into the model-level loose idles. From there, we could then 
      // trigger the IDLE to be moved into the correct place, when we're the ones who 
      // created the IDLE.
      // 
      // However, it's cleaner for us to have finer-grained control over this process. 
      // Since we can't slip in between the form being created and it being configured, 
      // the better option is:
      // 
      //  - Set a flag so that we ignore the next form-creation notification that we 
      //    get from DovahKitCore, such that we don't tell the datastore about the IDLE 
      //    we're creating.
      // 
      //  - Create the `idle_node` ourselves.
      // 
      //  - Use the datastore to insert the node. Since it has no parent node (not even 
      //    the model-level loose idle container node), the datastore will trigger the 
      //    insertion callbacks rather than the movement callbacks.
      // 
      // This function handles the first of those three steps for the specific case of 
      // duplicating an idle.
      //
      this->_callback_state.ignore_next_created_camera_path = true;
      auto request = DovahKitCore::get().request_form_creation(datastore_type::relevant_form_type);
      request.editorID = editor_id.toStdString();
      try {
         auto* stub = request.commit();
         if (!stub) {
            this->_callback_state.ignore_next_created_camera_path = false;
         }
         return stub;
      } catch (...) {
         this->_callback_state.ignore_next_created_camera_path = false;
         throw;
      }
   }
   dovah::form_stub* CameraPathFormsModel::_try_silently_duplicate_camera_path(dovah::form_stub& idle) {
      //
      // See documentation comment in `_try_silently_create_camera_path`.
      //

      auto& editor = DovahKitCore::get();

      dovah::form_stub* stub = nullptr;

      this->_callback_state.ignore_next_created_camera_path = true;
      try {
         auto request = editor.request_form_duplication();
         request.set_target(&idle);
         request.editorID = editor_helpers::make_editor_id_for_duplicate(idle.get_editor_id()).toStdString();
         stub = request.commit();
      } catch (...) {
         this->_callback_state.ignore_next_created_camera_path = false;
         throw;
      }
      if (!stub) {
         this->_callback_state.ignore_next_created_camera_path = false;
         return nullptr;
      }
      return stub;
   }
#pragma endregion
#pragma region Node utils
   bool CameraPathFormsModel::_can_move(const datastore_node& subject, const datastore_item& destination) const {
      return this->_datastore.is_node_movement_legal(subject, destination, nullptr);
   }
   void CameraPathFormsModel::_unchecked_move(datastore_node& subject, datastore_item& destination, int row) {
      assert(_can_move(subject, destination));
      QModelIndex subject_qmi     = _qmi_for_node(subject);
      QModelIndex destination_qmi = _qmi_for_item(destination);

      datastore_node* previous = nullptr;
      if (row < 0) {
         if (!destination.children.empty())
            previous = destination.children.back();
      } else if (row > 0) {
         previous = destination.children[row - 1];
      }

      // the datastore fires callbacks that trigger beforeMoveRows/endMoveRows when 
      // we tell it to move the node, so we shouldn't fire those here.

      this->_datastore.move_camera_path(subject, destination, previous);
   }
#pragma endregion

#pragma region Accessors
   QModelIndex CameraPathFormsModel::index(dovah::form_stub& stub) const noexcept {
      if (stub.form_type != datastore_type::relevant_form_type)
         return {};
      auto* node = this->_datastore.node_by_stub(stub);
      if (!node)
         return {};
      return _qmi_for_node(*node);
   }

   QModelIndex CameraPathFormsModel::createCameraPath(const QModelIndex& parent_qmi, QString idle_editor_id) {
      datastore_item* parent_item = _item_for_qmi(parent_qmi);
      if (!parent_item)
         return {};

      dovah::form_stub* stub = this->_try_silently_create_camera_path(idle_editor_id);
      if (!stub)
         return {};

      datastore_node* previous = nullptr;
      if (!parent_item->children.empty())
         previous = parent_item->children.back();

      auto  i_node_ptr = std::make_unique<datastore_node>(this->_datastore, *stub);
      auto* i_node     = i_node_ptr.get();
      assert(i_node != nullptr);
      this->_datastore.move_camera_path(*i_node, *parent_item, previous);
      assert(i_node->parent != nullptr);
      i_node_ptr.release();
      return _qmi_for_node(*i_node);
   }

   QModelIndex CameraPathFormsModel::createCameraPathAfter(const QModelIndex& sibling_qmi, QString idle_editor_id) {
      datastore_node* previous = _node_for_qmi(sibling_qmi);
      if (!previous)
         return {};
      datastore_item* parent = previous->parent;
      if (!parent)
         return {};

      dovah::form_stub* stub = this->_try_silently_create_camera_path(idle_editor_id);
      if (!stub)
         return {};

      auto  i_node_ptr = std::make_unique<datastore_node>(this->_datastore, *stub);
      auto* i_node     = i_node_ptr.get();
      assert(i_node != nullptr);
      this->_datastore.move_camera_path(*i_node, *parent, previous);
      assert(i_node->parent != nullptr);
      i_node_ptr.release();
      return _qmi_for_node(*i_node);
   }

   QModelIndex CameraPathFormsModel::duplicateCameraPath(const QModelIndex& idle_qmi, bool and_descendants) {
      auto* src_node = _node_for_qmi(idle_qmi);
      if (!src_node)
         return {};

      auto& editor = DovahKitCore::get();
      if (!editor.has_data())
         return {};

      if (and_descendants) {
         //
         // Ensure there are sufficient form IDs available in the active file.
         //
         {
            auto* flo = editor.get_file_load_order();
            if (!flo)
               return {};

            size_t count_to_duplicate = [](this auto&& recurse, datastore_node& idle) -> size_t {
               size_t count = 1;
               for (auto& child_ptr : idle.children) {
                  count += recurse(*child_ptr);
               }
               return count;
            }(*src_node);

            dovah::bare_form_id_t last_found_form_id     = 0;
            bool                  all_form_ids_available = true;
            size_t count_available = 0;
            for (size_t i = 0; i < count_to_duplicate; ++i) {
               auto id = flo->find_first_free_form_id_in_active_file(last_found_form_id);
               if (id == 0) {
                  all_form_ids_available = false;
                  break;
               }
               ++count_available;
               last_found_form_id = id;
            }
            if (!all_form_ids_available) {
               throw too_many_to_duplicate_exception(count_to_duplicate, count_available);
            }
         }
         //
         // Recursively duplicate the IDLEs.
         //
         assert(src_node->parent);
         QModelIndex root_qmi = {};
         [this, &root_qmi](this auto&& recurse, datastore_node& idle, datastore_item& dst_parent, bool is_root = false) -> void {
            dovah::form_stub* stub = this->_try_silently_duplicate_camera_path(idle.stub);
            if (!stub)
               return;

            auto  copy_ptr = std::make_unique<datastore_node>(this->_datastore, *stub);
            auto* copy     = copy_ptr.get();
            assert(copy != nullptr);
            if (is_root) {
               this->_datastore.move_camera_path(
                  *copy,
                  dst_parent,
                  &idle
               );
            } else {
               datastore_node* previous_idle = nullptr;
               if (!dst_parent.children.empty())
                  previous_idle = dst_parent.children.back();
               this->_datastore.move_camera_path(
                  *copy,
                  dst_parent,
                  previous_idle
               );
            }
            copy_ptr.release();
            if (is_root)
               root_qmi = _qmi_for_node(*copy);

            for (auto& child_ptr : idle.children) {
               recurse(*child_ptr, *copy);
            }
         }(*src_node, *src_node->parent, true);
         return root_qmi;
      } else {
         dovah::form_stub* stub = this->_try_silently_duplicate_camera_path(src_node->stub);
         if (!stub)
            return {};

         auto  copy_ptr = std::make_unique<datastore_node>(this->_datastore, *stub);
         auto* copy     = copy_ptr.get();
         assert(copy != nullptr);
         this->_datastore.move_camera_path(
            *copy,
            *src_node->parent,
            src_node
         );
         copy_ptr.release();
         return _qmi_for_node(*copy);
      }
   }

   void CameraPathFormsModel::deleteCameraPath(const QModelIndex& idle_qmi, QWidget* error_dialog_parent) {
      auto* src_node = dynamic_cast<datastore_node*>(_node_for_qmi(idle_qmi));
      if (!src_node)
         return;

      this->_handler_state.any_deletions_failed = false;
      this->_datastore.delete_camera_path(*src_node);
      if (this->_handler_state.any_deletions_failed) {
         this->_handler_state.any_deletions_failed = false;
         QMessageBox::critical(
            error_dialog_parent,
            QObject::tr("Error", "delete form error"),
            QObject::tr("Unable to delete all of the needed idles.")
         );
      }
   }

   bool CameraPathFormsModel::canMoveUp(const QModelIndex& idle_qmi) const {
      auto* subject = _node_for_qmi(idle_qmi);
      if (!subject)
         return false;
      if (!subject->parent)
         return false;

      size_t i = subject->parent->index_of_child(*subject);
      assert(i != datastore_node::index_of_none);
      return i > 0;
   }
   bool CameraPathFormsModel::canMoveDown(const QModelIndex& idle_qmi) const {
      auto* subject = _node_for_qmi(idle_qmi);
      if (!subject)
         return false;
      const datastore_item* parent = subject->parent;
      if (!parent)
         return false;

      size_t i = parent->index_of_child(*subject);
      assert(i != datastore_node::index_of_none);
      return i < parent->children.size() - 1;
   }
   void CameraPathFormsModel::moveUp(const QModelIndex& idle_qmi) {
      auto* node = _node_for_qmi(idle_qmi);
      if (!node)
         return;
      this->_datastore.move_camera_path_within_parent(*node, -1);
   }
   void CameraPathFormsModel::moveDown(const QModelIndex& idle_qmi) {
      auto* node = _node_for_qmi(idle_qmi);
      if (!node)
         return;
      this->_datastore.move_camera_path_within_parent(*node, 1);
   }
#pragma endregion

#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex CameraPathFormsModel::index(int row, int column, const QModelIndex& parent_qmi) const /*override*/ {
         if (row < 0 || column < 0)
            return {};
         const datastore_item* parent_item = _item_for_qmi(parent_qmi);
         if (!parent_item)
            return {};
         if (row >= parent_item->children.size())
            return {};
         return this->createIndex(row, column, (quintptr)parent_item);
      }
      /*virtual*/ QModelIndex CameraPathFormsModel::parent(const QModelIndex& index) const /*override*/ {
         if (!index.isValid())
            return {};
         const datastore_node* subject = _node_for_qmi(index);
         const datastore_item* parent  = subject->parent;
         if (parent)
            return _qmi_for_item(*parent);
         return {};
      }
      /*virtual*/ QModelIndex CameraPathFormsModel::sibling(int row, int column, const QModelIndex& index) const /*override*/ {
         if (row < 0 || column < 0)
            return {};
         datastore_item* parent = (datastore_item*)index.internalPointer();
         if (!parent) {
            if (row == 0)
               return _qmi_for_model_root();
            return {};
         }
         if (row >= parent->children.size())
            return {};
         return this->createIndex(row, column, (quintptr)parent);
      }
      /*virtual*/ int CameraPathFormsModel::rowCount(const QModelIndex& parent) const /*override*/ {
         const auto* node = _item_for_qmi(parent);
         return node->children.size();
      }
      /*virtual*/ int CameraPathFormsModel::columnCount(const QModelIndex& parent) const /*override*/ {
         return 1;
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant CameraPathFormsModel::data(const QModelIndex& qmi, int role) const /*override*/ {
         const datastore_node* node = _node_for_qmi(qmi);
         if (!node)
            return {};
         switch (role) {
            case FormStubRole:
               return QVariant::fromValue(&node->stub);
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
               {
                  auto it = this->_cache.find((datastore_node*)node); // can't use a const pointer to look up a non-const-pointer key -_-
                  if (it != this->_cache.end()) {
                     return it->second.display_string;
                  }
               }
               break;
         }
         return {};
      }
      /*virtual*/ Qt::ItemFlags CameraPathFormsModel::flags(const QModelIndex& index) const /*override*/ {
         auto  flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled;
         auto* node  = _node_for_qmi(index);
         if (!node) {
            return flags;
         }
         flags |= Qt::ItemIsDragEnabled;
         flags |= Qt::ItemIsDropEnabled;
         return flags;
      }
   #pragma endregion
   /*virtual*/ QVariant CameraPathFormsModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
      if (orientation == Qt::Orientation::Horizontal && section == 0) {
         switch (role) {
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
               return tr("Camera Paths");
         }
      }
      return {};
   }
   #pragma region Drag and drop
      #pragma region Whole-model queries
         /*virtual*/ QStringList CameraPathFormsModel::mimeTypes() const /*override*/ {
            return { QString::fromLatin1(mime_type) };
         }
         /*virtual*/ Qt::DropActions CameraPathFormsModel::supportedDropActions() const /*override*/ {
            return Qt::DropAction::CopyAction | Qt::DropAction::MoveAction;
         }
      #pragma endregion
      /*virtual*/ QMimeData* CameraPathFormsModel::mimeData(const QModelIndexList& indices) const /*override*/ {
         if (indices.count() <= 0)
            return nullptr;
         QByteArray  data;
         QDataStream stream(&data, QIODevice::WriteOnly);
         //
         // Stream begins with our `this` pointer. The pointer is used only for equality 
         // checks on drop (i.e. no moving/copying procedure data across packages) and is 
         // never dereferenced.
         //
         stream << (intptr_t)this;
         //
         for (const QModelIndex& qmi : indices) {
            const auto* node = _node_for_qmi(qmi);
            if (!node)
               continue;
            //
            // QAbstractItemModel's default implementation serializes all itemData for the 
            // node into the stream. We can't do that because some things, like conditions, 
            // are both not serializable *and* require references to objects held elsewhere. 
            // If QAbstractItemModel's implementation is designed to avoid the possibility 
            // of referenced objects being deleted during the drag operation, then trying 
            // to serialize conditions fails that requirement.
            // 
            // We also can't track the lifetime of the drag operation, so we can't, for 
            // example, store a map of unique IDs to QPersistentModelIndexes, because we 
            // wouldn't know when to destroy the QPMIs.
            // 
            // Our solution is to create IDs on demand for nodes that are being dragged, 
            // and serialize those. We never expose direct access to nodes (and thus to the 
            // unique_ptr list of child nodes), so all node removals go through us; we can 
            // invalidate unique IDs properly.
            // 
            // Of course, since we can't track the lifetime of a drag operation, we have to 
            // assume that a request for a node's MIME data is the start of a drag, and we 
            // have to create the ID then. Since the mimeData() getter is const, this is... 
            // a complication.
            //
            stream << this->_drag_and_drop.track(*const_cast<datastore_node*>(node));
         }
         //
         QMimeData* mime = new QMimeData();
         mime->setData(mime_type, data);
         return mime;
      }
      /*virtual*/ bool CameraPathFormsModel::canDropMimeData(const QMimeData* mime, Qt::DropAction action, int row, int column, const QModelIndex& parent) const /*override*/ {
         QByteArray  data = mime->data(mime_type);
         QDataStream stream(&data, QIODevice::ReadOnly);
         {  // Verify that this is an internal move.
            std::intptr_t this_pointer;
            stream >> this_pointer;
            if ((CameraPathFormsModel*)this_pointer != this)
               return false;
         }

         const datastore_item* parent_node = _item_for_qmi(parent);
         std::vector<datastore_node*> dragged_nodes;
         while (!stream.atEnd()) {
            DragDropTracking::uid_t id;
            stream >> id;
            auto* node = this->_drag_and_drop.get_by_id(id);
            if (node)
               dragged_nodes.push_back(node);
         }
         if (!dragged_nodes.size())
            return true;

         for (auto* dragged : dragged_nodes)
            if (!_can_move(*dragged, *parent_node))
               return false;

         return true;
      }
      /*virtual*/ bool CameraPathFormsModel::dropMimeData(const QMimeData* mime, Qt::DropAction action, int row, int column, const QModelIndex& parent) /*override*/ {
         if (!this->canDropMimeData(mime, action, row, column, parent))
            return false;

         QByteArray  data = mime->data(mime_type);
         QDataStream stream(&data, QIODevice::ReadOnly);
         {  // Verify that this is an internal move.
            std::intptr_t this_pointer;
            stream >> this_pointer;
            if ((CameraPathFormsModel*)this_pointer != this)
               return false;
         }
         std::vector<datastore_node*> nodes;
         while (!stream.atEnd()) {
            DragDropTracking::uid_t id;
            stream >> id;
            auto* node = this->_drag_and_drop.get_by_id(id);
            if (node)
               nodes.push_back(node);
         }
         if (!nodes.size())
            return false;

         auto* destination_parent = this->_item_for_qmi(parent);
         if (!destination_parent)
            destination_parent = &this->_datastore.root;
         for (auto it = nodes.rbegin(); it != nodes.rend(); ++it)
            this->_unchecked_move(**it, *destination_parent, row);
         return true;
      }
   #pragma endregion
#pragma endregion

#pragma region CameraPathFormsModel::DragDropTracking
   CameraPathFormsModel::DragDropTracking::uid_t CameraPathFormsModel::DragDropTracking::track(datastore_node& node) {
      for (const auto& pair : this->nodes)
         if (pair.second == &node)
            return pair.first;
      auto id = this->next_id;
      this->next_id++;
      this->nodes[id] = &node;
      return id;
   }
   void CameraPathFormsModel::DragDropTracking::untrack(datastore_node& node) {
      auto& map = this->nodes;
      auto  it  = std::find_if(map.begin(), map.end(), [&node](const auto& pair) {
         return pair.second == &node;
      });
      if (it != map.end())
         map.erase(it);
   }
   void CameraPathFormsModel::DragDropTracking::clear() {
      this->nodes.clear();
   }
   CameraPathFormsModel::datastore_node* CameraPathFormsModel::DragDropTracking::get_by_id(uid_t id) {
      auto& map = this->nodes;
      auto  it  = map.find(id);
      if (it != map.end())
         return it->second;
      return nullptr;
   }
#pragma endregion