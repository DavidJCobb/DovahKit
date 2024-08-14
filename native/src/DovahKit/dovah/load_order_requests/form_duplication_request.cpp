#include "./form_duplication_request.h"
#include "../files/file_load_order.h"
#include "../form_stubs/helpers/for_each_child_form.h"
#include "../form_stubs/helpers/for_each_quest_topic.h"
#include "./form_creation_request.h"
#include "../exceptions/form_creation_failed.h"

namespace dovah {
   form_duplication_request::form_duplication_request(file_load_order& o) : owner(o) {
   }
   form_duplication_request::form_duplication_request(form_duplication_request&& other) : owner(other.owner) {
      this->main_request = other.main_request;
      other.main_request = nullptr;
      //
      this->child_requests.clear();
      size_t size = other.child_requests.size();
      this->child_requests.resize(size);
      for(size_t i = 0; i < size; ++i) {
         this->child_requests[i] = other.child_requests[i];
         other.child_requests[i] = nullptr;
      }
   }
   form_duplication_request::~form_duplication_request() {
      for (auto* request : this->child_requests)
         if (request)
            delete request;
      this->child_requests.clear();
   }
   //
   void form_duplication_request::set_target(form_stub* original) {
      if (this->main_request && original == this->main_request->clone_of)
         return;
      //
      for (auto* request : this->child_requests)
         if (request)
            delete request;
      this->child_requests.clear();
      //
      if (this->main_request) {
         delete this->main_request;
         this->main_request = nullptr;
      }
      if (!original)
         return;
      //
      this->main_request = new form_creation_request(this->owner.request_form_creation(original->form_type));
      this->main_request->set_parent_form(this->parent);
      this->main_request->queue_clone(original);
      //
      form_stub_helpers::for_each_child_form(original, [this](form_stub* child) {
         auto* request = new form_creation_request(this->owner.request_form_creation(child->form_type));
         request->queue_clone(child);
         this->child_requests.push_back(request);
         return false;
      });
      if (original->form_type == form_type::quest) {
         form_stub_helpers::for_each_quest_topic(original, [this](form_stub* child) {
            auto* request = new form_creation_request(this->owner.request_form_creation(child->form_type));
            request->queue_clone(child);
            this->child_requests.push_back(request);
            return false;
         });
      }
   }
   //
   void form_duplication_request::set_parent_form(form_stub* parent) {
      this->parent = parent;
      if (this->main_request)
         this->main_request->set_parent_form(parent);
   }
   void form_duplication_request::set_parent_form(bare_form_id_t parentID) {
      this->parent = this->owner.get_form(parentID);
      if (this->main_request)
         this->main_request->set_parent_form(this->parent);
   }
   //
   form_stub* form_duplication_request::commit() {
      using exception = exceptions::form_creation_failed;

      if (!this->main_request)
         return nullptr;
      this->main_request->editorID = this->editorID;
      this->main_request->cell_grid_coordinates = this->cell_grid_coordinates;
      if (!this->parent) {
         if (auto* parent = this->main_request->clone_of->get_parent_form())
            this->main_request->set_parent_form(parent);
      }

      {
         struct {
            exception::error_code  code;
            form_creation_request* request = nullptr;
         } primary_failure;
         size_t error_count = 0;
         size_t ids_failed_to_allocate = 0;
         
         auto ec = this->owner.would_form_creation_request_fail(*this->main_request);
         if (ec.has_value()) {
            ++error_count;
            if (ec.value() == exception::error_code::no_form_id_available)
               ++ids_failed_to_allocate;
            primary_failure.code    = ec.value();
            primary_failure.request = this->main_request;
         }
         for (auto* request : this->child_requests) {
            if (!request)
               continue;
            auto ec = this->owner.would_form_creation_request_fail(*request);
            if (ec.has_value()) {
               ++error_count;
               if (ec.value() == exception::error_code::no_form_id_available)
                  ++ids_failed_to_allocate;
               if (!primary_failure.request) {
                  primary_failure.code    = ec.value();
                  primary_failure.request = this->main_request;
               }
            }
         }
         
         if (primary_failure.request) {
            auto ex = exception(primary_failure.code, *primary_failure.request);
            ex.details.form_ids_needed  = this->get_total_form_count();
            ex.details.form_ids_missing = ids_failed_to_allocate;
            ex.details.failure_count    = error_count;
            throw ex;
         }
      }

      form_stub* main_created_form = nullptr;
      try {
         main_created_form = this->main_request->commit();
         assert(main_created_form != nullptr);
         for (auto* request : this->child_requests) {
            if (!request)
               continue;
            request->set_parent_form(main_created_form);
            request->commit();
         }
      } catch (const exception& ex) {
         assert(false && "We pre-flighted the sub-requests for this form duplication request, and they all passed. How did we end up throwing an exception when committing them?");
         throw; // re-throw
      }
      return main_created_form;
   }
   size_t form_duplication_request::get_total_form_count() const noexcept {
      if (!this->main_request)
         return 0;
      return 1 + this->child_requests.size();
   }
}