#pragma once

namespace cobb::concepts {
   namespace impl {
      template <class T, template<typename...> typename Template>
      struct _type_specialization_of {
         static constexpr const bool value = false;
      };

      template <template<typename...> typename Template, class... Args>
      struct _type_specialization_of<Template<Args...>, Template> {
         static constexpr const bool value = true;
      };
   }

   template<typename T, template<typename...> typename Template>
   concept type_specialization_of = impl::_type_specialization_of<T, Template>::value;
}