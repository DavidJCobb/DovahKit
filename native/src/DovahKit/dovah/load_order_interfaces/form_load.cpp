#include "./form_load.h"
#include "../files/tes_file_reading/file_loader.h"
#include "../files/file_load_order.h"
#include "../notices/base_form_load_warning.h"
#include "../detailed_notice.h"
#include "../form_stub.h"

namespace dovah::load_order_interfaces {
   void form_load::log_load_warning(const detailed_notice& input) {
      if (!input.is_defined())
         return;
      detailed_notice warning = input;
      warning.type    = detailed_notice::notice_type::warning;
      warning.context = detailed_notice::notice_context::on_demand_form_load;
      warning.modify_flag(detailed_notice::flag::is_winning_record, this->is_winning_record);
      //
      if (this->current_file && warning.cause_file.empty()) {
         warning.cause_file = this->current_file->get_filename();
         warning.set_flag(detailed_notice::flag::has_cause_file);
      }
      //
      this->owner._log_load_warning(warning);
   }
   void form_load::log_load_warning(notices::base_form_load_warning& notice) {
      notice.record_info.is_winning_record = this->is_winning_record;
      if (this->current_file && notice.record_info.source_file.empty()) {
         notice.record_info.source_file = this->current_file->get_filename();
      }

      this->owner._log_warning(notice);
   }
   void form_load::warn_if_ref_is_wrong_type(
      form_stub* target,
      form_type  desired,
      uint32_t   subrecord_signature
   ) {
      if (!target)
         return;
      if (desired == form_type::reference) {
         if (form_type_is_reference(target->form_type))
            return;
      } else {
         if (target->form_type == desired)
            return;
      }
      notices::form_load_warnings::form_reference_type_mismatch notice(
         const_cast<form_stub&>(this->target_stub), // TODO: clean things up so we don't need const cast lol
         *target,
         desired,
         subrecord_signature
      );
      this->log_load_warning(notice);
   }
   
   void form_load::warn_if_ref_is_wrong_type(form_stub* target, std::vector<form_type> desired, uint32_t subrecord_signature) {
      if (!target)
         return;
      for (auto ft : desired)
         if (target->form_type == ft)
            return;

      notices::form_load_warnings::form_reference_type_mismatch notice(
         const_cast<form_stub&>(this->target_stub), // TODO: clean things up so we don't need const cast lol
         *target,
         form_type::none,
         subrecord_signature
      );
      notice.desired = desired;

      this->log_load_warning(notice);
   }

   void form_load::warn_on_unrecognized_subrecord(const tes_file_reading::subrecord& subrecord) {
      notices::form_load_warnings::unrecognized_subrecord notice(
         const_cast<form_stub&>(this->target_stub), // TODO: clean things up so we don't need const cast lol
         subrecord.signature()
      );
      this->log_load_warning(notice);
   }

   bool form_load::is_active_file() const noexcept {
      if (!this->current_file)
         return false;
      return this->owner.file_is_active(*this->current_file);
   }
}