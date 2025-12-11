#include "./story_manager_subsystem.h"
#include "../../core.h"
#include "dovah/datastores/story_manager/node.h"
#include "dovah/datastores/story_manager/branch_node.h"
#include "dovah/datastores/story_manager/leaf_node.h"

namespace dovahkit::subsystems::story_manager {
   core::core() {
      this->_datastore.callbacks.form_data_modified.before = [](dovah::form_stub& stub) {
         emit DovahKitCore::get().formModificationImminent(&stub);
      };
      this->_datastore.callbacks.form_data_modified.after = [](dovah::form_stub& stub) {
         emit DovahKitCore::get().formModified(&stub);
      };

      auto& editor = DovahKitCore::get();
      QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() {
         this->_datastore.reset();
      });
      QObject::connect(&editor, &DovahKitCore::dataAcquireComplete, this, &core::_rebuild_datastore);
      if (editor.has_data()) {
         this->_rebuild_datastore();
      }

      this->_datastore.handlers.delete_form = [this](dovah::form_stub& stub) {
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

      #pragma region Callbacks into signal emissions
         this->_datastore.callbacks.reset.before = [this]() { emit resetImminent(); };
         this->_datastore.callbacks.reset.after  = [this]() { emit resetComplete(); };
         
         this->_datastore.callbacks.node_deleted.before = [this](const node& a) { emit nodeDeletionImminent(a); };
         this->_datastore.callbacks.node_deleted.after  = [this](uint32_t form_id) { emit nodeDeletionComplete(); };
         
         this->_datastore.callbacks.node_placed.before = [this](const node& a, const branch_node& b, size_t c) { emit nodePlacementImminent(a, b, c); };
         this->_datastore.callbacks.node_placed.after  = [this](const node& a) { emit nodePlacementComplete(a); };
      #pragma endregion
   }

   void core::_rebuild_datastore() {
      auto* lo = DovahKitCore::get().get_file_load_order();
      if (!lo) {
         this->_datastore.reset();
         return;
      }
      this->_datastore.build(*lo);
      this->_datastore.normalize_for_editing();
      //
      // TODO: If we add warnings for the datastore to emit, then pass them to the Log Window here.
      //
   }

   void core::move_node(const node& subject, const branch_node& dst_parent, const node* dst_previous) {
      assert(&subject.datastore == &this->_datastore);
      assert(&dst_parent.datastore == &this->_datastore);
      if (dst_previous) {
         assert(&dst_previous->datastore == &this->_datastore);
      }
      this->_datastore.move_node(
         const_cast<node&>(subject),
         const_cast<branch_node&>(dst_parent),
         const_cast<node*>(dst_previous)
      );
   }
   void core::move_node_within_parent(const node& subject, int by) {
      assert(&subject.datastore == &this->_datastore);
      this->_datastore.move_node_within_parent(
         const_cast<node&>(subject),
         by
      );
   }
   bool core::delete_node(const node& subject) {
      assert(&subject.datastore == &this->_datastore);
      this->_datastore.delete_node(
         const_cast<node&>(subject)
      );

      bool failed = this->_handler_state.any_deletions_failed;
      this->_handler_state.any_deletions_failed = false;
      return !failed;
   }
}