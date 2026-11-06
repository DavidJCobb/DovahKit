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
   void _base_managed_localized_string_use::_discard_localized_id(form_stub& containing_form, localization::category category) {
      auto& alo         = containing_form.get_load_order();
      auto& lsc         = alo.get_localized_string_collection();
      auto* active_file = alo.get_active_file();
      assert(!!active_file);
      assert(!active_file->header().flags.has_all_of<tes::file_header_flag::localized_strings>() && "If the active file currently uses localized strings, we can't write a non-localized string in!");

      bare_localized_string_id prior_id = 0;
      if (std::holds_alternative<_localized_id>(this->_value)) {
         auto& v = std::get<_localized_id>(this->_value);
         auto* prior_file = v.file;
         auto  prior_id   = v.id;
         if (prior_id) {
            containing_form._replace_string_use({}, category, *prior_file, prior_id, 0);
         }
      }
   }

   void _base_managed_localized_string_use::_set_localized_id(form_stub& containing_form, localization::category category, bare_localized_string_id id) {
      auto& alo         = containing_form.get_load_order();
      auto& lsc         = alo.get_localized_string_collection();
      auto* active_file = alo.get_active_file();
      assert(!!active_file);
      assert(active_file->header().flags.has_all_of<tes::file_header_flag::localized_strings>() && "If the active file doesn't currently use localized strings, we can't write a localized string ID in!");

      const tes::file*         prior_file = nullptr;
      bare_localized_string_id prior_id   = 0;
      if (std::holds_alternative<_localized_id>(this->_value)) {
         auto& v = std::get<_localized_id>(this->_value);
         prior_file = v.file;
         prior_id   = v.id;
      }
      if (prior_id == id)
         return;
      containing_form._replace_string_use({}, category, *prior_file, prior_id, id);
      this->_value.emplace<_localized_id>() = {
         .file = active_file,
         .id   = id,
      };
   }
   void _base_managed_localized_string_use::_set_non_localized_string(form_stub& containing_form, localization::category category, std::string_view text) {
      this->_discard_localized_id(containing_form, category);
      this->_value.emplace<std::string>() = text;
   }
   void _base_managed_localized_string_use::_set_non_localized_string(form_stub& containing_form, localization::category category, std::string&& text) {
      this->_discard_localized_id(containing_form, category);
      this->_value.emplace<std::string>() = std::move(text);
   }

   void _base_managed_localized_string_use::_clear(form_stub& containing_form, localization::category category) {
      auto& alo         = containing_form.get_load_order();
      auto& lsc         = alo.get_localized_string_collection();
      auto* active_file = alo.get_active_file();
      assert(!!active_file);
      if (this->is_localized()) {
         if (std::holds_alternative<_localized_id>(this->_value)) {
            auto& v = std::get<_localized_id>(this->_value);
            auto* prior_file = v.file;
            auto  prior_id   = v.id;
            if (prior_id) {
               containing_form._replace_string_use({}, category, *prior_file, prior_id, 0);
            }
         }
      }
      if (active_file->header().flags.has_all_of<tes::file_header_flag::localized_strings>()) {
         auto& v = this->_value.emplace<_localized_id>();
         v.file = active_file;
         v.id   = 0;
      } else {
         this->_value.emplace<std::string>();
      }
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