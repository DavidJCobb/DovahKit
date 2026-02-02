#include "./condition_list.h"
#include <array>
#include <bitset>
#include <concepts>
#include <utility> // std::unreachable
#include "../../forms/_all.h"

namespace {
   using component_type = dovah::loaded_forms::components::condition_list;
}

namespace {
   template<typename T>
   concept has_component = requires(T & x) {
      { x.conditions } -> std::same_as<component_type&>;
   };

   template<has_component T>
   component_type* get_component(dovah::loaded_forms::Form& form) {
      return &static_cast<T&>(form).conditions;
   }
   using getter_type = component_type* (*)(dovah::loaded_forms::Form&);

   constexpr const size_t forms_with_component_count = [](){
      size_t count = 0;
      dovah::all_loaded_form_types::for_each([&count]<typename Form>() {
         if constexpr (has_component<Form>)
            ++count;
      });
      return count;
   }();

   //
   // The naive approach would be to create a function table, indexed by the form 
   // type. However, the vast majority of form types won't have every component. 
   // There are over 0x80 form types, so on x64, with eight bytes per pointer, 
   // that will result in a kilobyte of space that's almost entirely nullptr.
   // 
   // A smarter approach is to create a map of form types to getters. The naive 
   // approach to the smart approach is to have a struct containing a single 
   // form type and a single getter function pointer, and to then generate an 
   // array of these structs. We can then walk the array to look up the getter. 
   // However, a form-type value is a single byte, so on x64, we end up with 
   // seven padding bytes per entry: 43.75% of the space in each entry will be 
   // wasted.
   // 
   // So what if, instead, we have a list of batches? Each batch can contain 
   // as many form types as will fit in the space taken up by a pointer, and 
   // then an equivalent number of pointers. So on x64, we have eight form 
   // types and then eight getter function pointers, per entry. That should 
   // minimize the amount of padding.
   // 
   // Of course, the best approach would be to somehow generate a switch-case 
   // on the form type, but I don't know how to reliably do that through 
   // templates alone. There are designs that a compiler's optimizer may perhaps 
   // be able to recognize and convert into such a switch-case (e.g. one form 
   // type and one getter per entry), but there's nothing in the language that 
   // would be *certain* to produce such an outcome. Maybe in the future, we 
   // could use `template for` to produce an if/else tree with branches only 
   // for form types that have the component; that, I think, a compiler ought 
   // to always be able to recognize and optimize.
   //

   static constexpr const size_t form_types_per_batch = sizeof(void*) / sizeof(dovah::form_type);
   struct mapping_batch {
      std::array<dovah::form_type, form_types_per_batch> form_types;
      std::array<getter_type,      form_types_per_batch> getters;
   };
   constexpr auto mapping_list = []() {
      constexpr const size_t batch_count =
         (forms_with_component_count / form_types_per_batch) +
         (forms_with_component_count % form_types_per_batch ? 1 : 0)
      ;
      std::array<mapping_batch, batch_count> batches = {};
      {
         size_t bi = 0;
         size_t fi = 0;
         dovah::all_loaded_form_types::for_each([&batches, &bi, &fi]<typename Form>() {
            if constexpr (has_component<Form>) {
               auto& batch = batches[bi];
               batch.form_types[fi] = Form::form_type;
               batch.getters[fi]    = &get_component<Form>;
               if (++fi >= form_types_per_batch) {
                  ++bi;
                  fi = 0;
               }
            }
         });
      }
      return batches;
   }();

   constexpr auto presence = []() {
      std::bitset<dovah::form_types.size()> presence;
      dovah::all_loaded_form_types::for_each([&presence]<typename Form>() {
         if constexpr (has_component<Form>)
            presence.set((size_t)Form::form_type);
      });
      return presence;
   }();
}

namespace dovah::utils::form_component_accessors {
   extern const loaded_forms::components::condition_list* condition_list(const dovah::loaded_forms::Form& form) {
      return condition_list(const_cast<dovah::loaded_forms::Form&>(form));
   }
   extern loaded_forms::components::condition_list* condition_list(dovah::loaded_forms::Form& form) {
      const size_t i = (size_t)form.stub.form_type;
      if (!presence.test(i))
         return nullptr;
      for (const auto& batch : mapping_list)
         for (size_t i = 0; i < form_types_per_batch; ++i)
            if (form.stub.form_type == batch.form_types[i])
               return batch.getters[i](form);
      std::unreachable();
      return nullptr;
   }
}