#pragma once
#include "./DKFormVMADModel.h"

template<typename ScriptFunctor, typename PropFunctor>
bool DKFormVMADModel::handleIndexByType(QModelIndex qmi, ScriptFunctor&& sf, PropFunctor&& pf) {
   if (!qmi.isValid() || qmi.model() != this)
      return false;

   const void* ptr = qmi.internalPointer();
   for (size_t i = 0; i < this->scripts.size(); ++i) {
      auto* script = this->scripts[i];
      if (script == ptr) {
         sf(qmi, script);
         return true;
      }
      for (auto* prop : script->properties) {
         if (prop == ptr) {
            pf(qmi, prop);
            return true;
         }
      }
   }
   return false;
}

template<typename ScriptFunctor, typename PropFunctor>
bool DKFormVMADModel::handleIndexByType(QModelIndex qmi, ScriptFunctor&& sf, PropFunctor&& pf) const {
   if (!qmi.isValid() || qmi.model() != this)
      return false;

   const void* ptr = qmi.internalPointer();
   for (size_t i = 0; i < this->scripts.size(); ++i) {
      const auto* script = this->scripts[i];
      if (script == ptr) {
         sf(qmi, script);
         return true;
      }
      for (const auto* prop : script->properties) {
         if (prop == ptr) {
            pf(qmi, prop);
            return true;
         }
      }
   }
   return false;
}
