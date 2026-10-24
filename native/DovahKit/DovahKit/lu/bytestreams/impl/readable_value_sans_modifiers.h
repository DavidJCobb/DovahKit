#pragma once
#include "./innermost_kind_of.h"

namespace lu::bytestreams::impl {
   template<typename T, typename Stream>
   concept readable_value_sans_modifiers = innermost_kind_of<T, Stream> != value_kind::not_serializable;
}