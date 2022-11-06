#include "tool_options_base.h"

namespace DK3DToolOptions {
   void Base::setControlType(control_type ct) {
      if (ct == this->state.controlType)
         return;
      this->state.controlType = ct;
      emit this->controlTypeChanged(ct);
   }
   void Base::showOptions(const opaque_option_union& oou) {
      if (oou.id() != this->state.id)
         return;
      this->_readOptions(oou);
   }
   void Base::writeTo(opaque_option_union& oou) {
      if (oou.id() != this->state.id)
         return;
      this->_writeOptions(oou);
   }
}