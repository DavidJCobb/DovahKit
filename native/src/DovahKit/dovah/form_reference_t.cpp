#include "./form_reference_t.h"
#include "./form_stub.h"
#include "./forms/Form.h"

namespace dovah {
   bare_form_id_t form_reference_t::formID() const noexcept {
      return this->stub ? this->stub->formID : 0;
   }
   void form_reference_t::clear_if(loaded_forms::Form& owner, form_stub& clear_if) {
      if (this->stub == &clear_if)
         this->set(owner, nullptr);
   }
   void form_reference_t::set(loaded_forms::Form& owner, form_stub* set_to) {
      if (this->stub == set_to)
         return;
      if (!owner.is_working_copy) {
         bare_form_id_t old = this->stub ? this->stub->formID : 0;
         owner.stub.replace_outbound_reference(old, set_to, this->use_info_flags);
      }
      this->stub = set_to;
   }
   void form_reference_t::set(loaded_forms::Form& owner, const form_reference_t& set_to) {
      this->set(owner, set_to.stub);
   }
   bool form_reference_t::form_type_matches(form_type ft) const noexcept {
      if (!this->stub)
         return true;
      return this->stub->form_type == ft;
   }
   void form_reference_t::unmanaged_set(form_stub* set_to) {
      this->stub = set_to;
   }
   
   void clear_form_reference_list(std::vector<form_reference_t>& list, loaded_forms::Form& owner) {
      for (auto& id : list)
         id.set(owner, nullptr);
      list.clear();
   }
   void remove_form_from_reference_list(std::vector<form_reference_t>& list, form_stub& target, loaded_forms::Form& owner) {
      bool edits = false;
      for (auto& id : list) {
         if (id == &target) {
            id.set(owner, nullptr);
            edits = true;
         }
      }
      if (edits) {
         list.erase(
            std::remove_if(list.begin(), list.end(), [](const form_reference_t& id) { return id == nullptr; }),
            list.end()
         );
      }
   }
   void copy_form_reference_list(loaded_forms::Form& target_owner, std::vector<form_reference_t>& target, const std::vector<form_reference_t>& source) {
      size_t size = source.size();
      if (target.size() < size)
         target.resize(size);
      for (size_t i = 0; i < size; ++i)
         target[i].set(target_owner, source[i]);
      if (target.size() > size) {
         for (size_t i = size; i < target.size(); ++i)
            target[i].set(target_owner, nullptr);
         target.resize(size);
      }
   }
}