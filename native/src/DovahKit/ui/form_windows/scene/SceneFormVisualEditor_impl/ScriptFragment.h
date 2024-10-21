#pragma once
#include <string>
#include <QString>

namespace SceneFormVisualEditor_impl {
   struct ScriptFragment {
      public:
         std::string scriptname;
         std::string function;

      public:
         void setFromQt(QString sn, QString fn);
         void toQt(QString& dst_sn, QString& dst_fn) const;

         constexpr bool empty() const noexcept {
            if (!this->scriptname.empty())
               return false;
            if (!this->function.empty())
               return false;
            return true;
         }
   };
}