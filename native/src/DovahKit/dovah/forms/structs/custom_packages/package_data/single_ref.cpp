#include "./single_ref.h"
#include <memory>

namespace dovah::loaded_forms::structs::custom_packages {
   /*virtual*/ package_data* package_data_single_ref::clone(loaded_forms::Form& owner_of_clone) const noexcept /*override*/ {
      auto  copy_ptr = std::make_unique<package_data_single_ref>();
      auto* copy = copy_ptr.get();

      copy->data.clone_from(this->data, owner_of_clone);

      return copy_ptr.release();
   }
}