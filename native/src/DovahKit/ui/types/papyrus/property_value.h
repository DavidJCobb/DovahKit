#pragma once
#include <variant>
#include <vector>
#include <QString>
#include "helpers/type_traits/is_std_vector.h"

#include "dovah/forms/components/papyrus/property_value.h"

#include "../quest_alias.h"
#include "./value_type.h"

namespace dovah {
   class form_reference_t;
   class form_stub;
}

namespace ui::types::papyrus {
   using property_value = std::variant<
      std::monostate, // only for clearing an inherited property value REFR-side

      // Single-value types:

      bool,
      float,
      int32_t,
      QString,
      //
      ui::types::quest_alias,
      dovah::form_stub*,

      // Arrays of the above:

      std::vector<bool>,
      std::vector<float>,
      std::vector<int32_t>,
      std::vector<QString>,
      //
      std::vector<ui::types::quest_alias>,
      std::vector<dovah::form_stub*>
   >;

   constexpr value_type value_type_of(const property_value&);

   constexpr property_value property_value_from_type(value_type);

   //
   // Convert a Papyrus property value, as stored in form data, to the UI-side representation.
   // 
   // If it's an "object" or "array of object" property, then you have to know the underlying native 
   // type (as specified in the compiled script file that defines the property), i.e. a form type or 
   // alias type. For the purposes of this function, `dovah::form_type::none` maps to `Form`; use an 
   // empty optional to indicate a type that doesn't subclass any native type.
   // 
   // When a native type is specified,  object values that don't match will be converted to None, as 
   // will non-object values.  When no native type is specified, object values  will be converted to 
   // none.
   // 
   property_value vmad_property_value_to_ui(const dovah::loaded_forms::components::papyrus::property_value&, std::optional<dovah::form_type_t> underlying_native_type);

   //
   // Convert a Papyrus property value, as stored in the UI-side representation, to form data.
   //
   void ui_property_value_to_vmad(const property_value& src, dovah::loaded_forms::components::papyrus::property_value& dst, dovah::loaded_forms::Form& dst_owner);
}

#include "./property_value.inl"