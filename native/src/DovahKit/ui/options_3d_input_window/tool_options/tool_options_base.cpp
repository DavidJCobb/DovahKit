#include "tool_options_base.h"

namespace DK3DToolOptions {
   void Base::setControlType(DK3D::ControlType ct) {
      if (ct == this->state.controlType)
         return;
      this->state.controlType = ct;
      emit this->controlTypeChanged(ct);
   }
   void Base::showOptions(const DK3D::tools::opaque_option_union& oou) {
      if (oou.id() != this->state.id)
         return;
      this->_readOptions(oou);
   }
   void Base::writeTo(DK3D::tools::opaque_option_union& oou) {
      if (oou.id() != this->state.id)
         return;
      this->_writeOptions(oou);
   }
}