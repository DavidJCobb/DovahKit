#include "./form_duplication_request.h"
#include "../files/file_load_order.h"
#include "../form_stub_helpers.h"
#include "./form_creation_request.h"

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
      if (!this->main_request)
         return nullptr;
      this->main_request->editorID = this->editorID;
      this->main_request->cell_grid_coordinates.x = this->cell_grid_coordinates.x;
      this->main_request->cell_grid_coordinates.y = this->cell_grid_coordinates.y;
      this->main_request->cell_grid_coordinates.present = this->cell_grid_coordinates.present;
      if (!this->parent) {
         if (auto* parent = this->main_request->clone_of->get_parent_form())
            this->main_request->set_parent_form(parent);
      }
      auto* result = this->main_request->commit();
      if (!result)
         return nullptr;
      //
      for (auto* request : this->child_requests) {
         if (!request)
            continue;
         request->set_parent_form(result);
         request->commit();
      }
      //
      return result;
   }
   notice_code_t form_duplication_request::get_main_form_error_code() const noexcept {
      if (this->main_request)
         return this->main_request->error;
      return default_notice_code;
   }
   std::vector<notice_code_t> form_duplication_request::get_child_form_error_codes() const noexcept {
      std::vector<notice_code_t> out;
      for (auto* request : this->child_requests)
         if (request && request->error != default_notice_code)
            out.push_back(request->error);
      return out;
   }
   std::vector<notice_code_t> form_duplication_request::get_error_codes() const noexcept {
      std::vector<notice_code_t> out;
      if (this->main_request) {
         out.push_back(this->main_request->error);
         for (auto* request : this->child_requests)
            if (request)
               out.push_back(request->error);
      }
      return out;
   }
   bool form_duplication_request::has_error() const noexcept {
      if (this->main_request) {
         if (this->main_request->error != default_notice_code)
            return true;
         for (auto* request : this->child_requests)
            if (request && request->error != default_notice_code)
               return true;
      }
      return false;
   }
   bool form_duplication_request::is_valid() const noexcept {
      if (!this->main_request)
         return false;
      return this->main_request->is_valid();
   }
   size_t form_duplication_request::get_total_form_count() const noexcept {
      if (!this->main_request)
         return 0;
      return 1 + this->child_requests.size();
   }
}