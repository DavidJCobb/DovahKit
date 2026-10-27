#include "./form_load.h"
#include "dovah/notices/base_form_load_warning.h"
#include "dovah/notices/form_load_warnings/unrecognized_subrecord.h"
#include "dovah/tes/file.h"
#include "dovah/tes/reading/subrecord_view.h"
#include "dovah/active_load_order.h"
#include "dovah/form_stub.h"
#include "dovah/form_stub_collection.h"
#include "dovah/types/managed_form_use.h"

#include "dovah/notices/form_load_warnings/form_use_is_wrong_type.h"

namespace dovah::load_order_interfaces {
   form_load::form_load(form_stub& t) : active_load_order(t.get_load_order()), target_stub(t) {
   }

   bool form_load::is_active_file() const noexcept {
      if (!this->current_file)
         return false;
      return this->current_file == this->active_load_order.get_active_file();
   }

   void form_load::log_load_warning(notices::base_form_load_warning&& notice) {
      if (this->is_during_file_save_cleanup) {
         return;
      }
      const auto callback = this->active_load_order.post_load_callbacks.on_warning;
      if (!callback)
         return;

      notice.record_info.is_winning_record = this->is_winning_record;
      if (this->current_file && notice.record_info.source_filename.empty()) {
         notice.record_info.source_filename = this->current_file->path().filename().string();
      }

      (callback)(notice);
   }

   void form_load::_warn_if_use_is_wrong_type_impl(const managed_form_use& use, const tes::subrecord_view& subrecord, const form_type* types, size_t type_count) {
      auto* stub = use.get();
      if (!stub)
         return;
      for (size_t i = 0; i < type_count; ++i)
         if (stub->form_type == types[i])
            return;
      this->log_load_warning(
         notices::form_load_warnings::form_use_is_wrong_type(
            this->target_stub,
            *stub,
            std::vector<form_type>(types, types + type_count),
            subrecord.signature()
         )
      );
   }
   void form_load::_warn_if_use_is_wrong_type_impl(const managed_form_use& use, const tes::subrecord_view& subrecord, size_t nth_subrecord, const form_type* types, size_t type_count) {
      auto* stub = use.get();
      if (!stub)
         return;
      for (size_t i = 0; i < type_count; ++i)
         if (stub->form_type == types[i])
            return;
      this->log_load_warning(
         notices::form_load_warnings::form_use_is_wrong_type(
            this->target_stub,
            *stub,
            std::vector<form_type>(types, types + type_count),
            subrecord.signature(),
            {
               .nth_subrecord = nth_subrecord,
            }
         )
      );
   }

   void form_load::warn_if_use_is_wrong_type(const managed_form_use& use, form_type required, const tes::subrecord_view& subrecord) {
      _warn_if_use_is_wrong_type_impl(use, subrecord, &required, 1);
   }
   void form_load::warn_if_use_is_wrong_type(const managed_form_use& use, form_type required, const tes::subrecord_view& subrecord, size_t nth_subrecord) {
      _warn_if_use_is_wrong_type_impl(use, subrecord, nth_subrecord, &required, 1);
   }
   void form_load::warn_on_unrecognized_subrecord(const tes::subrecord_view& subrecord) {
      this->log_load_warning(notices::form_load_warnings::unrecognized_subrecord(
         this->target_stub,
         subrecord.signature()
      ));
   }
}