#pragma once
#include <memory>
#include "dovah/active_load_order.h"

namespace dovahkit::tests::pieces {
   extern std::unique_ptr<dovah::active_load_order> load_skyrim_esm(bool break_on_warnings);
}