#pragma once
#include <array>
#include <cstdint>

namespace dovah::loaded_forms::structs {
   struct actor_facegen_morphs {
      public:
         static constexpr const int32_t no_morph_index = -1;

         enum class indexed_morph_type {
            nose,
            brows,
            eyes,
            mouth
         };

      protected:
         // NPC_/NAMA subrecord.
         union indexed_morph_list {
            public:
               using value_type = int32_t;

               constexpr indexed_morph_list() {}
               constexpr indexed_morph_list(const indexed_morph_list& o) : list(o.list) {}
               constexpr ~indexed_morph_list() { list.~array(); }

            protected:
               std::array<value_type, 4> list = { 0, no_morph_index, 0, 0 };

               using array_type = decltype(list);

            public:
               struct {
                  value_type nose;
                  value_type brows; // not used in vanilla nor exposed in the CK
                  value_type eyes;
                  value_type lips;
               };

            public:
               constexpr value_type& operator[](size_t t) noexcept { return list[(size_t)t]; }
               constexpr const value_type& operator[](size_t t) const noexcept { return list[(size_t)t]; }
               constexpr value_type& operator[](indexed_morph_type t) noexcept { return list[(size_t)t]; }
               constexpr const value_type& operator[](indexed_morph_type t) const noexcept { return list[(size_t)t]; }

               constexpr array_type::iterator begin() { return list.begin(); }
               constexpr array_type::iterator end() { return list.end(); }
               constexpr array_type::const_iterator begin() const { return list.begin(); }
               constexpr array_type::const_iterator end() const { return list.end(); }
               constexpr size_t size() const noexcept { return list.size(); }
         };

         // NPC_/NAM9 subrecord. Only serialized if any slider is non-zero.
         union slider_list {
            public:
               using value_type = float;

               constexpr slider_list() {}
               constexpr slider_list(const slider_list& o) : list(o.list) {}
               constexpr ~slider_list() { list.~array(); }

            protected:
               std::array<value_type, 19> list = {};

               using array_type = decltype(list);
               
            public:
               struct {
                  struct {
                     value_type length;
                     value_type height;
                  } nose;
                  struct {
                     value_type height;
                     value_type width;
                     value_type depth;
                  } jaw;
                  struct {
                     value_type height;
                     value_type width;
                  } cheeks;
                  struct {
                     value_type height;
                     value_type width;
                     value_type depth; // read AFTER chin
                  } eyes;
                  struct {
                     value_type height;
                     value_type width;
                     value_type depth;
                  } brows;
                  struct {
                     value_type height;
                     value_type depth;
                  } mouth;
                  struct {
                     value_type width;
                     value_type height;
                     value_type depth;
                  } chin;
                  value_type vampire_morph;
               };

            public:
               constexpr value_type& operator[](size_t t) noexcept { return list[(size_t)t]; }
               constexpr const value_type& operator[](size_t t) const noexcept { return list[(size_t)t]; }

               constexpr array_type::iterator begin() { return list.begin(); }
               constexpr array_type::iterator end() { return list.end(); }
               constexpr array_type::const_iterator begin() const { return list.begin(); }
               constexpr array_type::const_iterator end() const { return list.end(); }
               constexpr size_t size() const noexcept { return list.size(); }
         };
         static_assert(offsetof(slider_list, cheeks) == sizeof(slider_list::value_type) * 5, "The anonymous structs must not be padded.");

      public:
         indexed_morph_list indices;
         slider_list        sliders;

         constexpr bool all_sliders_zeroed() const noexcept {
            for (auto f : this->sliders)
               if (f)
                  return false;
            return true;
         }
   };
}