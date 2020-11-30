#include "detailed_notice.h"
#include "form_stub.h"
#include "notice_code_list.h"

namespace dovah {
   void detailed_notice::add_relevant_form(const form_stub& stub) {
      auto& entry = this->relevant_forms.emplace_back();
      entry.type    = stub.formType;
      entry.fixedID = stub.formID;
      entry.localID = 0;
   }

   /*static*/ detailed_notice detailed_notice::warn_about_unrecognized_subrecord(uint32_t subrecord, const form_stub& referrer) {
      detailed_notice warning;
      warning.code = notice_code::unrecognized_subrecord;
      warning.set_cause_form(referrer);
      warning.set_cause_subrecord(subrecord);
      return warning;
   }
   /*static*/ detailed_notice detailed_notice::warn_if_wrong_type(uint32_t subrecord_signature, form_type_t desired, const form_stub& referrer, const form_reference_t& reference) {
      if (reference.form_type_matches(desired))
         return detailed_notice();
      detailed_notice warning;
      warning.code = notice_code::form_reference_is_of_incorrect_type;
      warning.set_cause_form(referrer);
      warning.set_cause_form_type(desired);
      warning.set_cause_subrecord(subrecord_signature);
      warning.add_relevant_form(*reference.get_form_stub());
      return warning;
   }
   /*static*/ detailed_notice detailed_notice::warn_if_wrong_type(uint32_t subrecord_signature, std::initializer_list<form_type_t> desired, const form_stub& referrer, const form_reference_t& reference) {
      for (auto ft : desired)
         if (reference.form_type_matches(ft))
            return detailed_notice();
      detailed_notice warning;
      warning.code = notice_code::form_reference_is_of_incorrect_type;
      warning.set_cause_form(referrer);
      warning.set_cause_subrecord(subrecord_signature);
      warning.add_relevant_form(*reference.get_form_stub());
      return warning;
   }
   /*static*/ detailed_notice detailed_notice::warn_if_not_object_reference(uint32_t subrecord_signature, const form_stub& referrer, const form_reference_t& reference) {
      for (auto& info : form_types)
         if (info.is_reference())
            if (reference.form_type_matches(info.formType))
               return detailed_notice();
      detailed_notice warning;
      warning.code = notice_code::form_reference_is_of_incorrect_type;
      warning.set_cause_form(referrer);
      warning.set_cause_subrecord(subrecord_signature);
      warning.add_relevant_form(*reference.get_form_stub());
      return warning;
   }


   detailed_notice& detailed_notice::set_cause_form_index(int i) noexcept {
      this->cause_form_index = i;
      this->flags |= flag::has_cause_form_index;
      return *this;
   }
   detailed_notice& detailed_notice::set_file_offset(uint32_t o) noexcept {
      this->offset = o;
      this->flags |= flag::has_file_offset;
      return *this;
   }
   detailed_notice& detailed_notice::set_subrecord_index(int i) noexcept {
      this->cause_subrecord_index = i;
      this->flags |= flag::has_cause_subrecord_index;
      return *this;
   }
   detailed_notice& detailed_notice::set_cause_form(const form_stub& stub) {
      this->cause_form.type    = stub.formType;
      this->cause_form.fixedID = stub.formID;
      this->cause_form.localID = 0;
      this->set_flag(flag::has_cause_form);
      return *this;
   }
   detailed_notice& detailed_notice::set_cause_form_type(form_type_t ft) {
      this->cause_form_type = ft;
      this->set_flag(flag::has_cause_form_type);
      return *this;
   }
   detailed_notice& detailed_notice::set_cause_subrecord(uint32_t signature) {
      this->cause_subrecord = signature;
      this->set_flag(flag::has_cause_subrecord);
      return *this;
   }
   detailed_notice& detailed_notice::set_cause_editor_id(const std::string& ed) {
      this->cause_editor_id = ed;
      if (!ed.empty())
         this->set_flag(flag::has_cause_editor_id);
      return *this;
   }

   bool detailed_notice::operator==(const detailed_notice& other) const noexcept {
      if (this->type != other.type)
         return false;
      if (this->context != other.context)
         return false;
      if (this->code != other.code)
         return false;
      if (this->flags != other.flags)
         return false;
      if (this->flags & flag::has_file_offset)
         if (this->offset != other.offset)
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
      if (this->flags & flag::has_cause_editor_id)
         if (this->cause_editor_id != other.cause_editor_id)
            return false;
      if (this->cause_file != other.cause_file)
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