#pragma once
#include <concepts>
#include <string>
#include <string_view>
#include <vector>

namespace cobb {
   namespace interned_string_tables {
      // Template parameters for `interned_string_table`. You can subclass this to override just 
      // specific options.
      struct default_parameters {
         using string_type = std::string;
         using view_type   = std::string_view;

         // Case folding is available as an optional feature. If you want it, you have to supply the 
         // folding logic (e.g. what character sets to fold over, etc.).

         // If true, `fold` is used on strings before they're stored, such that all strings in the 
         // table are folded. If false, then strings aren't folded on storage, so the first case of 
         // a string to be stored becomes "canonical."
         static constexpr const bool fold_on_store = true;

         // Used to perform case-folding on a string when storing it in the table.
         static constexpr void fold(string_type& out) {
         }

         // Used to compare two strings for equality.
         //
         // If you want case-folding to happen, but DON'T want it to happen when strings are stored, 
         // then this function must compare two unfolded strings for case-insensitive equality.
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

   //
   // Simple system for string interning. Only supports adding strings; the only way to remove 
   // strings is to clear the whole string table.
   // 
   // Add strings to the table by calling `get_or_insert`, which will return a string_view to 
   // the copy of the string that the table owns. You can optionally enable case-folding and 
   // other options by changing the template parameter.
   // 
   // Class is set up so that if you're not folding on store, then we only heap-allocate when 
   // adding strings to the list; string_views are used for as many operations as possible.
   //
   template<interned_string_tables::valid_parameters Parameters = interned_string_tables::default_parameters>
   class interned_string_table {
      protected:
         using template_parameters = Parameters;

      public:
         using string_type = typename template_parameters::string_type;
         using view_type = typename template_parameters::view_type;

      protected:

         //
         // I'm fairly sure that when a std::vector<std::string> needs to reallocate on insertion, 
         // it should move the strings instead of copying them, such that the memory addresses of 
         // each element's [i.e. string's] buffer remain unchanged. For whatever reason, however, 
         // in MSVC Debug, that isn't happening: the strings are copied instead, which changes the 
         // addresses each string's actual content is stored at. I've looked at documentation for 
         // the classes online and dug through the MSVC STL vector implementation and I don't know 
         // why that's happening; the vector implementation actively tries to prevent it.
         // 
         // When that happens, it breaks our implementation, because we depend on storing single 
         // copies of each string in an append-only table, and passing around string views. These 
         // views should remain valid for as long as the interned string table exists, even if we 
         // add more strings to the table afterward. When std::vector botches things and copies 
         // all the strings rather than moving them, that has the effect of relocating the strings 
         // in memory and leaving all extant string_views dangling.
         // 
         // For now, we'll work around it by allocating and freeing the strings ourselves, with a 
         // vector of non-owning string_views. Set this to `false` if we can ever use real strings 
         // safely.
         //
         static constexpr const bool compiler_copies_vec_elems_during_realloc = true;

         using string_list_type = std::conditional_t<
            compiler_copies_vec_elems_during_realloc,
            std::vector<view_type>,
            std::vector<string_type>
         >;

      public:
         using string_type = typename template_parameters::string_type;
         using view_type   = typename template_parameters::view_type;

      protected:
         string_list_type _storage;

         constexpr view_type _copy_and_store(const view_type& src) requires compiler_copies_vec_elems_during_realloc {
            size_t size    = src.size();
            auto*  content = new typename string_type::value_type[size];
            if (std::is_constant_evaluated()) {
               for (size_t i = 0; i < size; ++i)
                  content[i] = src[i];
            } else {
               memcpy(content, src.data(), size);
            }
            try {
               return _storage.emplace_back(content, size);
            } catch (...) {
               delete content; // avoid leaks on vector realloc failure
               throw; // re-throw caught exception
            }
         }

      public:
         constexpr ~interned_string_table() {
            this->clear();
         }

         // WARNING: Will delete strings out from any extant views.
         constexpr void clear() {
            if constexpr (compiler_copies_vec_elems_during_realloc) {
               for (view_type& view : this->_storage)
                  delete[] view.data();
            }
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
               if constexpr (compiler_copies_vec_elems_during_realloc) {
                  return _copy_and_store(subject);
               } else {
                  auto& stored = _storage.emplace_back(std::move(subject));
                  return view_type(stored);
               }
            } else {
               if constexpr (compiler_copies_vec_elems_during_realloc) {
                  return _copy_and_store(data);
               } else {
                  auto& stored = _storage.emplace_back(data);
                  return view_type(stored);
               }
            }
         }

         //

         constexpr const auto& all_stored() const { return _storage; }

         constexpr size_t count_stored() const { return _storage.size(); }
   };
}
