#pragma once
#include <concepts>
#include <string>
#include <string_view>
#include <vector>

namespace cobb {
   namespace interned_string_tables {
      struct default_parameters {
         using string_type = std::string;
         using view_type   = std::string_view;

         static constexpr const bool fold_on_store = true;

         static constexpr void fold(string_type& out) {
         }
         static constexpr bool are_equal(const view_type& a, const view_type& b) {
            return a == b;
         }
      };

      template<typename Params>
      concept valid_parameters = requires {
         typename Params::string_type;
         typename Params::view_type;
         { Params::fold_on_store } -> std::same_as<const bool&>;
         requires requires(typename Params::string_type& s, const typename Params::view_type& v) {
            { Params::fold(s) };
            { Params::are_equal(v, v) } -> std::same_as<bool>;
         };
      };
   }

   template<interned_string_tables::valid_parameters Parameters = interned_string_tables::default_parameters>
   class interned_string_table {
      protected:
         using template_parameters = Parameters;

      public:
         using string_type = typename template_parameters::string_type;
         using view_type   = typename template_parameters::view_type;

      protected:
         std::vector<string_type> _storage;

      public:
         // WARNING: Will delete strings out from any extant views.
         constexpr void clear() {
            this->_storage.clear();
         }

         constexpr bool contains(const view_type& data) {
            for (const auto& item : _storage)
               if (template_parameters::are_equal(data, item))
                  return true;
            return false;
         }

         constexpr view_type get_or_insert(const view_type& data) {
            for (const auto& item : _storage)
               if (template_parameters::are_equal(data, item))
                  return view_type(item);
            if constexpr (template_parameters::fold_on_store) {
               string_type subject(data);
               template_parameters::fold(subject);
               auto& stored = _storage.emplace_back(std::move(subject));
               return view_type(stored);
            } else {
               auto& stored = _storage.emplace_back(data);
               return view_type(stored);
            }
         }

         //

         constexpr const auto& all_stored() const { return _storage; }

         constexpr size_t count_stored() const { return _storage.size(); }
   };
}
