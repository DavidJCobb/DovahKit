#include "./story_manager_subsystem.h"
#include "../../core.h"
#include "dovah/datastores/story_manager/node.h"
#include "dovah/datastores/story_manager/branch_node.h"
#include "dovah/datastores/story_manager/leaf_node.h"
#include "./passkeys/core_controls_model.h"
#include "./StoryManagerFormsModel.h"

namespace dovahkit::subsystems::story_manager {
   core::core() {
      this->_model = new StoryManagerFormsModel({}, this);

      this->_datastore.callbacks.form_data_modified.before = [](dovah::form_stub& stub) {
         emit DovahKitCore::get().formModificationImminent(&stub);
      };
      this->_datastore.callbacks.form_data_modified.after = [](dovah::form_stub& stub) {
         emit DovahKitCore::get().formModified(&stub);
      };

      auto& editor = DovahKitCore::get();
      QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() {
         this->_model->_on_before_datastore_reset({});
         this->_datastore.reset();
         this->_model->_on_after_datastore_reset({}, false);
      });
      QObject::connect(&editor, &DovahKitCore::dataAcquireComplete, this, &core::_rebuild_datastore);
      if (editor.has_data()) {
         this->_rebuild_datastore();
      }
      QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) {
         switch (stub->form_type) {
            case dovah::form_type::story_branch_node:
            case dovah::form_type::story_event_node:
            case dovah::form_type::story_quest_node:
            case dovah::form_type::quest:
               this->_model->_on_form_modified({}, *stub);
               break;
         }
      });
      QObject::connect(&editor, &DovahKitCore::formCreated, this, [this](dovah::form_stub* stub) {
         this->_datastore.on_form_created(*stub);
      });

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
         this->_datastore.callbacks.reset.before = [this]() {
            this->_model->_on_before_datastore_reset({});
         };
         this->_datastore.callbacks.reset.after  = [this]() {
            this->_model->_on_after_datastore_reset({}, false);
         };
         
         this->_datastore.callbacks.node_deleted.before = [this](const node& subject) {
            this->_model->_on_node_deletion_imminent({}, subject);
         };
         this->_datastore.callbacks.node_deleted.after  = [this](uint32_t form_id) {
            this->_model->_on_node_deletion_complete({});
         };
         
         this->_datastore.callbacks.node_placed.before = [this](const node& subject, const branch_node& dst_parent, size_t dst_pos) {
            this->_model->_on_node_placement_imminent({}, subject, dst_parent, dst_pos);
         };
         this->_datastore.callbacks.node_placed.after  = [this](const node& subject) {
            this->_model->_on_node_placement_complete({}, subject);
         };
      #pragma endregion
   }
   core::~core() {
   }

   void core::_rebuild_datastore() {
      this->_model->_on_before_datastore_reset({});
      auto* lo = DovahKitCore::get().get_file_load_order();
      if (!lo) {
         this->_datastore.reset();
         this->_model->_on_after_datastore_reset({}, false);
         return;
      }
      this->_datastore.build(*lo);
      this->_datastore.normalize_for_editing();
      this->_model->_on_after_datastore_reset({}, true);
      //
      // TODO: If we add warnings for the datastore to emit, then pass them to the Log Window here.
      //
   }

   dovah::form_stub* core::containing_event_node_of(const dovah::form_stub& stub) const noexcept {
      switch (stub.form_type) {
         case dovah::form_type::story_branch_node:
         case dovah::form_type::story_quest_node:
            break;
         default:
            return nullptr;
      }
      auto* node = this->_datastore.node_by_stub(stub);
      if (!node)
         return nullptr;
      while (node = node->parent)
         if (node->stub.form_type == dovah::form_type::story_event_node)
            return &node->stub;
      return nullptr;
   }
   std::optional<dovah::story_event_code::type> core::event_type_for(const dovah::form_stub& stub) const noexcept {
      if (stub.form_type != dovah::form_type::story_event_node)
         return {};
      auto qmi  = this->_model->index(stub);
      auto data = this->_model->data(qmi, StoryManagerFormsModel::EventTypeRole);
      if (!data.isValid())
         return {};
      return (dovah::story_event_code::type)data.toInt();
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
   void core::move_node(dovah::form_stub& subject, dovah::form_stub& dst_parent, dovah::form_stub* dst_previous) {
      node* subj_node = this->_datastore.node_by_stub(subject);
      assert(subj_node != nullptr);
      auto* dstp_node = dynamic_cast<branch_node*>(this->_datastore.node_by_stub(dst_parent));
      assert(dstp_node != nullptr);
      node* prev_node = nullptr;
      if (dst_previous)
         prev_node = this->_datastore.node_by_stub(*dst_previous);
      assert(dstp_node != nullptr);
      this->_datastore.move_node(*subj_node, *dstp_node, prev_node);
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