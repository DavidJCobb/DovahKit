#pragma once

namespace lu::bytestreams::impl {
   enum class value_kind {
      not_serializable,
      serializable_scalar,
      serializable_struct,
   };
}