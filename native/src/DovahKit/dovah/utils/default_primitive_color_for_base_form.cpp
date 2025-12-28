#include "./default_primitive_color_for_base_form.h"
#include "../data/hardcoded_form_ids.h"
#include "../form_stub.h"

namespace dovah::utils {
   extern loaded_forms::color_floats default_primitive_color_for_base_form(const dovah::form_stub& stub) {
      switch (stub.formID) {
         case hardcoded_form_ids::CollisionMarker:
            return { 1.0F, 1.5F, 0.0F, 0.15F };
         case hardcoded_form_ids::MultiBoundMarker:
            return { 0.3F, 0.8F, 0.1F, 0.15F };
         case hardcoded_form_ids::PlaneMarker:
            return { 1.0F, 1.0F, 1.0F, 0.15F };
         case hardcoded_form_ids::PortalMarker:
            return { 0.0F, 0.0F, 0.0F, 0.25F };
         case hardcoded_form_ids::RoomMarker:
            return { 0.0F, 0.5F, 1.0F, 0.20F };
      }
      switch (stub.form_type) {
         case form_type::acoustic_space:
            return { 1.0F, 0.2F, 0.9F, 0.15F };
      }
      return { 0, 0, 0, 1 };
   }
}