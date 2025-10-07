#include "./filter_info.h"
#include "dovah/form_stub.h"
#include "editor/subsystems/form_info_cache/cacheable_traits/model_path.h"
#include "editor/subsystems/form_info_cache/core.h"

namespace {
   using form_info_cache = dovahkit::subsystems::form_info_cache::core;
}

namespace ui::object_window {
   bool filter_info::empty() const noexcept {
      if (!this->form_types.empty())
         return false;
      if (!this->filters.model_path_prefix.isEmpty())
         return false;
      if (!this->filters.quest_filter_prefix.isEmpty())
         return false;
      return true;
   }

   bool filter_info::operator==(const filter_info& other) const noexcept {
      if (this == &other)
         return true;

      if (this->filters.model_path_prefix != other.filters.model_path_prefix)
         return false;
      if (this->filters.quest_filter_prefix != other.filters.quest_filter_prefix)
         return false;

      if (this->form_types.size() != other.form_types.size()) {
         return false;
      }
      for (auto a : this->form_types) {
         bool found = false;
         for (auto b : other.form_types) {
            if (a == b) {
               found = true;
               break;
            }
         }
         if (!found)
            return false;
      }

      return true;
   }

   /*static*/ QString filter_info::normalize_pathlike_string(const QString& filter) noexcept {
      auto size = filter.size();
      
      QString out;
      out.reserve(size);
      
      QChar prev = '\0';
      for (decltype(size) i = 0; i < size; ++i) {
         QChar c = filter[i];
         if (c == '/' || c == '\\') {
            if (prev == '/' || prev == '\0') // simplify doubled slashes, and strip leading slashes
               continue;
            c = '/';
         }
         out += c;
         prev = c;
      }
      if (!out.isEmpty() && out.back() == '/') // strip trailing slashes
         out.resize(out.size() - 1);
      
      return out;
   }

   bool filter_info::form_matches_filters(const dovah::form_stub& stub) const noexcept {
      if (!this->form_types.contains(stub.form_type))
         return false;

      if (stub.form_type == dovah::form_type::none)
         return stub.is_none_stub();

      if (stub.form_type == dovah::form_type::quest) {
         const auto& filter = this->filters.quest_filter_prefix;
         if (filter.isEmpty())
            return true;
         const auto data = normalize_pathlike_string(form_info_cache::get().get_quest_filter(stub));
         return data.startsWith(filter);
      }

      if (dovahkit::subsystems::form_info_cache::cacheable_traits::model_path::form_type_is_of_interest(stub.form_type)) {
         const auto& filter = this->filters.model_path_prefix;
         if (filter.isEmpty())
            return true;
         const auto data = normalize_pathlike_string(form_info_cache::get().get_form_model_path(stub));
         return data.startsWith(filter);
      }

      // No applicable filters to test against.
      return true;
   }
}