#pragma once
#include <mutex>
#include <unordered_map>
#include "lu/foreach_lambda_result.h"
#include "lu/passkey.h"
#include "./types/bare_form_id.h"
#include "./form_type_info.h"
namespace dovah {
   namespace tes {
      class file;
   }
   class active_load_order;
   class form_stub;
}

namespace dovah {
   class form_stub_collection {
      public:
         form_stub_collection(const form_stub_collection&) = delete;
         form_stub_collection& operator=(const form_stub_collection&) = delete;

      protected:
         struct _form_map {
            mutable std::mutex lock; // used only during multi-threaded file indexing
            std::unordered_map<bare_form_id, form_stub*> forms;
         };

         struct _typed_map_set : public std::array<_form_map, form_types.size()> {
            protected:
               static constexpr const size_t form_types_are_contiguous_up_to = []() -> size_t {
                  for (size_t i = 0; i < form_types.size(); ++i)
                     if ((size_t)form_types[i].form_type != i)
                        return i;
                  return form_types.size();
               }();
               static constexpr const bool all_form_types_are_contiguous = form_types_are_contiguous_up_to == form_types.size();

            public:
               using array::operator[];

               constexpr _form_map& operator[](form_type ft) {
                  return const_cast<_form_map&>(std::as_const(*this).operator[](ft));
               }
               constexpr const _form_map& operator[](form_type ft) const {
                  if constexpr (all_form_types_are_contiguous) {
                     return operator[]((size_t)ft);
                  } else {
                     if ((size_t)ft < form_types_are_contiguous_up_to)
                        return operator[]((size_t)ft);
                     for (size_t i = form_types_are_contiguous_up_to; i < form_types.size(); ++i)
                        if (form_types[i].form_type == ft)
                           return operator[](i);
                     throw std::out_of_range("no form map exists for this form type");
                  }
               }

               static constexpr bool supports_form_type(form_type ft) {
                  if constexpr (all_form_types_are_contiguous) {
                     return (size_t)ft < form_types_are_contiguous_up_to;
                  } else {
                     if ((size_t)ft < form_types_are_contiguous_up_to)
                        return true;
                     for (size_t i = form_types_are_contiguous_up_to; i < form_types.size(); ++i)
                        if (form_types[i].form_type == ft)
                           return true;
                     return false;
                  }
               }
         };

      public:
         _form_map      forms;
         _typed_map_set forms_by_type;
         _form_map      active_file_forms; // forms defined OR overridden by the active file
         _typed_map_set active_file_forms_by_type;

      public:
         inline size_t count_forms_of_type(form_type) const noexcept;
         
         template<typename Functor>
            requires std::is_invocable_v<Functor, const form_stub&>
         void for_each_form(Functor&& functor) const;
         //
         template<typename Functor>
            requires std::is_invocable_v<Functor, form_stub&>
         void for_each_form(Functor&& functor);
         
         template<typename Functor>
            requires std::is_invocable_v<Functor, const form_stub&>
         void for_each_form_of_type(form_type, Functor&& functor) const;
         //
         template<typename Functor>
            requires std::is_invocable_v<Functor, form_stub&>
         void for_each_form_of_type(form_type, Functor&& functor);

         form_stub* get_canonical_instance_of_singleton_form(form_type) const noexcept;
         form_stub* get_or_create_canonical_instance_of_singleton_form(form_type);
         inline form_stub* get_form_by_id(bare_form_id);
         inline form_stub* get_form_by_id(bare_form_id, form_type);

      public: // passkeyed
         void _make_hardcoded_forms(lu::passkey<active_load_order>, tes::file& hardcoded_forms_file);
         void _on_stub_flagged_as_edited(lu::passkey<form_stub>, form_stub&);
   };
}

#include "./form_stub_collection.inl"