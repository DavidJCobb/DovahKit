#include "./managed_localized_string_use.h"
#include <stdexcept>
#include "dovah/active_load_order.h"
#include "dovah/form_stub.h"
#include "dovah/localization/string_collection.h"
#include "dovah/tes/file.h"
#include "dovah/tes/file_header_flags.h"
#include "dovah/tes/reading/file_view.h"
#include "dovah/tes/reading/subrecord_view.h"

namespace dovah {
   void _base_managed_localized_string_use::_discard_localized_id(form_stub& containing_form, localized_string_type type) {
      auto& alo         = containing_form.get_load_order();
      auto& lsc         = alo.get_localized_string_collection();
      auto* active_file = alo.get_active_file();
      assert(!!active_file);
      assert(!active_file->header().flags.has_all_of<tes::file_header_flag::localized_strings>() && "If the active file currently uses localized strings, we can't write a non-localized string in!");

      bare_localized_string_id prior_id = 0;
      if (std::holds_alternative<_localized_id>(this->_value)) {
         prior_id = std::get<_localized_id>(this->_value).id;
         if (prior_id) {
            containing_form._replace_active_file_string_use({}, type, prior_id, 0);
         }
      }
   }

   void _base_managed_localized_string_use::_set_localized_id(form_stub& containing_form, localized_string_type type, bare_localized_string_id id) {
      auto& alo         = containing_form.get_load_order();
      auto& lsc         = alo.get_localized_string_collection();
      auto* active_file = alo.get_active_file();
      assert(!!active_file);
      assert(active_file->header().flags.has_all_of<tes::file_header_flag::localized_strings>() && "If the active file doesn't currently use localized strings, we can't write a localized string ID in!");

      bare_localized_string_id prior_id = 0;
      if (std::holds_alternative<_localized_id>(this->_value)) {
         prior_id = std::get<_localized_id>(this->_value).id;
      }
      if (prior_id == id)
         return;
      containing_form._replace_active_file_string_use({}, type, prior_id, id);
      this->_value.emplace<_localized_id>() = {
         .file = active_file,
         .id   = id,
      };
   }
   void _base_managed_localized_string_use::_set_non_localized_string(form_stub& containing_form, localized_string_type type, std::string_view text) {
      this->_discard_localized_id(containing_form, type);
      this->_value.emplace<std::string>() = text;
   }
   void _base_managed_localized_string_use::_set_non_localized_string(form_stub& containing_form, localized_string_type type, std::string&& text) {
      this->_discard_localized_id(containing_form, type);
      this->_value.emplace<std::string>() = std::move(text);
   }

   bool _base_managed_localized_string_use::is_active_file_string_id() const noexcept {
      if (!this->is_localized())
         return false;
      auto& info = std::get<_localized_id>(this->_value);
      if (!info.file)
         return false;
      auto* alo = info.file->get_active_load_order();
      return (alo && info.file == alo->get_active_file());
   }

   bool _base_managed_localized_string_use::stream(tes::subrecord_view& subrecord) {
      const auto& source_file = subrecord.get_source_file();
      const bool  localized   = source_file.header().flags.has_all_of<tes::file_header_flag::localized_strings>();
      if (localized) {
         if (!std::holds_alternative<_localized_id>(this->_value))
            this->_value.emplace<_localized_id>();
         auto& v = std::get<_localized_id>(this->_value);
         if (subrecord.read(v.id)) {
            v.file = &source_file;
            return true;
         } else {
            return false;
         }
      } else {
         if (!std::holds_alternative<std::string>(this->_value))
            this->_value.emplace<std::string>();
         auto& v = std::get<std::string>(this->_value);
         subrecord.read_all_to_string(v);
         return true;
      }
   }
}