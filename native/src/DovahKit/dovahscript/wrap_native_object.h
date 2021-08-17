#pragma once
#include "../lua.h"
#include "wrapper.h"

class QObject;
namespace dovah {
   class form_stub;
}
namespace dovahscript {
   class DovahscriptResource;
   class DovahscriptResourceHandle;
}

namespace dovahscript {
   [[nodiscard]] extern wrapper wrap_native_object(dovah::form_stub&);

   [[nodiscard]] extern wrapper wrap_native_object(DovahscriptResource&);

   [[nodiscard]] extern wrapper wrap_native_object(DovahscriptResourceHandle);

   [[nodiscard]] extern wrapper wrap_native_object(QObject&);
}