#include "./component_accessors.h"
#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include "dovah/form_type_info.h"
#include "./form_type_for_form_data_class.h"

#include "./all_types.h"
#include "./components/bounds.h"

// Only suitable if the component type is not, itself, templated on the form-data-params.
// Requires that `component_type` be in-scope as the component type.
#define DEFINE_SIMPLE_CONCEPT_CHECK(member_name) \
   template<typename FormData> \
   concept has_component = requires(FormData& data) { \
      { data.member_name } -> std::same_as<component_type&>; \
   };

// Use if the compoennt type is templated on form-data-params.
// Requires that `component_type` be in-scope as the component type template.
#define DEFINE_PARAMS_CONCEPT_CHECK(member_name) \
   template<typename FormData> \
   concept has_component = requires(FormData& data) { \
      typename FormData::form_data_params; \
      { data.member_name } -> std::same_as<component_type<FormData::form_data_params>&>; \
   };

namespace dovah::form_data::component_accessors {
   #pragma region bounds
      namespace bounds_impl {
         using component_type = components::bounds;
         using offset_type    = uint16_t;

         DEFINE_SIMPLE_CONCEPT_CHECK(bounds);

         template<typename FormData>
         constexpr offset_type offset_in = []() {
            if constexpr (has_component<FormData>) {
               return offsetof(FormData, bounds);
            } else {
               return 0;
            }
         }();

         template<typename FormDataParams>
         constexpr const auto table = []() {
            std::array<offset_type, form_types.size()> offsets = { 0 };
            all_form_data_types<FormDataParams>::for_each([&offsets]<typename FD>() {
               offsets[(size_t)form_type_for_form_data_class_v<FD>] = offset_in<FD>;
            });
            return offsets;
         }();
      }
      const components::bounds* bounds(const base_managed_form_data& data) {
         auto offset = bounds_impl::table<managed_form_params>[(size_t)data.stub.form_type];
         if (!offset)
            return nullptr;
         return (const components::bounds*)((const char*)&data + offset);
      }
      components::bounds* bounds(base_managed_form_data& data) {
         return const_cast<components::bounds*>(bounds(std::as_const(data)));
      }
      const components::bounds* bounds(const base_unmanaged_form_data& data) {
         auto offset = bounds_impl::table<unmanaged_form_params>[(size_t)data.stub.form_type];
         if (!offset)
            return nullptr;
         return (const components::bounds*)((const char*)&data + offset);
      }
      components::bounds* bounds(base_unmanaged_form_data& data) {
         return const_cast<components::bounds*>(bounds(std::as_const(data)));
      }
   #pragma endregion
}