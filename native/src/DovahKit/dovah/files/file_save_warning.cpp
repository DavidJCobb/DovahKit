#include "file_save_warning.h"
#include "../form_stub.h"
#include "../notice_code_list.h"

namespace dovah {
   void file_save_warning::set_cause_form(const form_stub& stub) {
      this->cause_form.type    = stub.formType;
      this->cause_form.fixedID = stub.formID;
      this->set_flag(flag::has_cause_form);
   }
   void file_save_warning::set_cause_form_type(form_type_t ft) {
      this->cause_form_type = ft;
      this->set_flag(flag::has_cause_form_type);
   }
   void file_save_warning::set_cause_subrecord(uint32_t signature) {
      this->cause_subrecord = signature;
      this->set_flag(flag::has_cause_subrecord);
   }

   void file_save_warning::add_relevant_form(const form_stub& stub) {
      auto& entry = this->relevant_forms.emplace_back();
      entry.type    = stub.formType;
      entry.fixedID = stub.formID;
   }

   file_save_warning& file_save_warning::set_cause_form_index(int i) noexcept {
      this->cause_form_index = i;
      this->flags |= flag::has_cause_form_index;
      return *this;
   }
   file_save_warning& file_save_warning::set_subrecord_index(int i) noexcept {
      this->cause_subrecord_index = i;
      this->flags |= flag::has_cause_subrecord_index;
      return *this;
   }

   bool file_save_warning::operator==(const file_save_warning& other) const noexcept {
      if (this->code != other.code)
         return false;
      if (this->flags != other.flags)
         return false;
      if (this->flags & flag::has_cause_subrecord) {
         if (this->cause_subrecord != other.cause_subrecord)
            return false;
         if (this->flags & flag::has_cause_subrecord_index)
            if (this->cause_subrecord_index != other.cause_subrecord_index)
               return false;
      }
      if (this->flags & flag::has_cause_form)
         if (this->cause_form != other.cause_form)
            return false;
      if (this->flags & flag::has_cause_form_type)
         if (this->cause_form_type != other.cause_form_type)
            return false;
      if (this->relevant_forms != other.relevant_forms)
         return false;
      if (this->relevant_files != other.relevant_files)
         return false;
      if (this->extra_integers != other.extra_integers)
         return false;
      return true;
   }
}