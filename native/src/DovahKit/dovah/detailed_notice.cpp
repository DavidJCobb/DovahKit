#include "detailed_notice.h"
#include "form_stub.h"
#include "notice_code_list.h"

namespace dovah {
   void detailed_notice::add_relevant_form(const form_stub& stub) {
      auto& entry = this->relevant_forms.emplace_back();
      entry.type    = stub.form_type;
      entry.fixedID = stub.formID;
      entry.localID = 0;
   }


   detailed_notice& detailed_notice::set_cause_form_index(int i) noexcept {
      this->cause_form_index = i;
      this->flags |= flag::has_cause_form_index;
      return *this;
   }
   detailed_notice& detailed_notice::set_cause_signature(uint32_t s) noexcept {
      this->cause_signature = s;
      this->flags |= flag::has_cause_signature;
      return *this;
   }
   detailed_notice& detailed_notice::set_cause_size(uint32_t s) noexcept {
      this->cause_size = s;
      this->flags |= flag::has_cause_size;
      return *this;
   }
   detailed_notice& detailed_notice::set_errno(errno_t e) noexcept {
      this->errno_value = e;
      this->flags |= flag::has_errno;
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
   detailed_notice& detailed_notice::set_winapi_error_code(uint32_t c) noexcept {
      this->winapi_error = c;
      this->flags |= flag::has_winapi_error_code;
      return *this;
   }
   detailed_notice& detailed_notice::set_cause_file(const std::string& filename) {
      this->cause_file = filename;
      this->set_flag(flag::has_cause_file);
      return *this;
   }
   detailed_notice& detailed_notice::set_cause_form(const form_stub& stub) {
      this->cause_form.type    = stub.form_type;
      this->cause_form.fixedID = stub.formID;
      this->cause_form.localID = 0;
      this->set_flag(flag::has_cause_form);
      return *this;
   }
   detailed_notice& detailed_notice::set_cause_form_type(form_type ft) {
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

   detailed_notice& detailed_notice::add_relevant_file(const std::string& filename) {
      this->relevant_files.emplace_back(filename);
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