#pragma once
#include "../helpers/multiheap.h"
#include "form_stub.h"

namespace dovah {
   using form_stub_heap = cobb::multiheap<form_stub, 16000>;
}
