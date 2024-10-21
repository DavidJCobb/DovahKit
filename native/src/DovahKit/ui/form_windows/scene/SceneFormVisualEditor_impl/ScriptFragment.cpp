#include "./ScriptFragment.h"

namespace SceneFormVisualEditor_impl {
   void ScriptFragment::setFromQt(QString sn, QString fn) {
      auto sn8 = sn.toUtf8();
      auto fn8 = fn.toUtf8();

      this->scriptname.assign(sn8.data(), sn8.size());
      this->function.assign(fn8.data(), fn8.size());
   }
   void ScriptFragment::toQt(QString& dst_sn, QString& dst_fn) const {
      dst_sn = QString::fromUtf8(this->scriptname.data(), this->scriptname.size());
      dst_fn = QString::fromUtf8(this->function.data(), this->function.size());
   }
}