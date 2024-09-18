#pragma once
#include "./DKCustomFormFilter.h"

namespace dovah {
   class form_stub;
}

class DKCustomFormFilterableModelMixin {
   public:
      virtual void recheck_custom_filter_for_all_forms() = 0;
      virtual void recheck_custom_filter_for_form(dovah::form_stub&) = 0;

      DKCustomFormFilter* get_custom_filter() const;
      void set_custom_filter(DKCustomFormFilter*);

   protected:
      QPointer<DKCustomFormFilter> _custom_filter = nullptr;
};
