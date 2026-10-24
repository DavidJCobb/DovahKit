#pragma once
#include "lu/utils/nominal_integer_type.h"
#include "./bare_form_id.h"
namespace dovah::tes {
   class subrecord_view;
}

namespace dovah {
   class form_id : public lu::utils::nominal_integer_type<form_id, bare_form_id> {
      private:
         using _my_base = lu::utils::nominal_integer_type<form_id, bare_form_id>;
      public:
         using integral_type = _my_base::integral_type;
         using _my_base::nominal_integer_type;

         void unchecked_stream(tes::subrecord_view&);
   };
}

namespace lu::bytestreams {
   //
   // Since we're declaring `unchecked_stream`, that type now counts as a serializable struct, so its 
   // serialized size doesn't auto-default to that of the wrapped integral type. We need to explicitly 
   // customize the serialized size for our type.
   //
   template<typename Stream, typename T>
   struct serialized_bytecount;
   //
   template<typename Stream>
   struct serialized_bytecount<Stream, dovah::form_id> {
      static constexpr const size_t value = serialized_bytecount<Stream, utils::integral_wrapper_underlying_type<dovah::form_id>>::value;
   };
}
