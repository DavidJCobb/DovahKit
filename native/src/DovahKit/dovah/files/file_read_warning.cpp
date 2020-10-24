#include "file_read_warning.h"
#include "../form_stub.h"
#include "../notice_code_list.h"

namespace dovah {
   void file_read_warning::set_cause_form(const form_stub& stub) {
      this->cause_form.type    = stub.formType;
      this->cause_form.fixedID = stub.formID;
      this->cause_form.localID = 0;
      this->set_flag(flag::has_cause_form);
   }
   void file_read_warning::set_cause_form_type(form_type_t ft) {
      this->cause_form_type = ft;
      this->set_flag(flag::has_cause_form_type);
   }
   void file_read_warning::set_cause_subrecord(uint32_t signature) {
      this->cause_subrecord = signature;
      this->set_flag(flag::has_cause_subrecord);
   }
   void file_read_warning::add_relevant_form(const form_stub& stub) {
      auto& entry = this->relevant_forms.emplace_back();
      entry.type    = stub.formType;
      entry.fixedID = stub.formID;
      entry.localID = 0;
   }

   /*static*/ file_read_warning file_read_warning::warn_about_unrecognized_subrecord(uint32_t subrecord, const form_stub& referrer) {
      file_read_warning warning;
      warning.code = notice_code::unrecognized_subrecord;
      warning.set_cause_form(referrer);
      warning.set_cause_subrecord(subrecord);
      return warning;
   }
   /*static*/ file_read_warning file_read_warning::warn_if_wrong_type(uint32_t subrecord_signature, form_type_t desired, const form_stub& referrer, const form_reference_t& reference) {
      if (reference.form_type_matches(desired))
         return file_read_warning();
      file_read_warning warning;
      warning.code = notice_code::form_reference_is_of_incorrect_type;
      warning.set_cause_form(referrer);
      warning.set_cause_form_type(desired);
      warning.set_cause_subrecord(subrecord_signature);
      warning.add_relevant_form(*reference.get_form_stub());
      return warning;
   }
}