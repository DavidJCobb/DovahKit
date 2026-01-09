#pragma once
#include <cstdint>
#include <optional>
#include <variant>
#include "../../../data/region_data_type.h"
#include "./generable_content/audio.h"
#include "./generable_content/grass.h"
#include "./generable_content/landscape.h"
#include "./generable_content/map.h"
#include "./generable_content/objects.h"
#include "./generable_content/weather.h"
namespace dovah {
   namespace loaded_forms {
      class Form;
   }
   class form_stub;
}

namespace dovah::loaded_forms::structs::region {
   class generable_content_collection {
      public:
         bool    override; // RDAT+0x04
         uint8_t priority; // RDAT+0x05
         std::variant< // indices map to the RDAT+0x00 "type" enum
            std::monostate, // unknown/unused
            std::monostate, // unknown/unused
            generable_content::raw_object_collection,
            generable_content::weather_collection,
            generable_content::map,
            generable_content::landscape,
            generable_content::grass_collection,
            generable_content::audio
         > data;

      public:
         template<typename T>
         T* as() noexcept {
            return std::get_if<T>(&this->data);
         }

         template<typename T>
         const T* as() const noexcept {
            return std::get_if<T>(&this->data);
         }

         void clear(dovah::loaded_forms::Form& containing_form);
         void clone_from(dovah::loaded_forms::Form& my_containing_form, const generable_content_collection& src);
         void sever_references_to(dovah::loaded_forms::Form& containing_form, dovah::form_stub&);

         template<typename T>
         T& get_or_emplace(dovah::loaded_forms::Form& containing_form) {
            if (std::holds_alternative<T>(this->data))
               return std::get<T>(this->data);
            this->clear(containing_form);
            return this->data.emplace<T>();
         }

         constexpr std::optional<region_data_type> type() const noexcept {
            auto i = this->data.index();
            if (i >= 2 && i <= 7) {
               return (region_data_type)(i - 2);
            }
            return {};
         }
   };
}