#pragma once
#include <memory>
#include <vector>
#include <QMetaType> // Q_DECLARE_METATYPE
#include "dovah/forms/structs/region/generable_content/objects.h"
namespace dovah {
   namespace loaded_forms {
      namespace structs::region::generable_content {
         class raw_object_collection;
      }
      class Region;
   }
   class form_stub;
   enum class form_type : uint8_t;
}

namespace ui::types::regions::generable_content {
   struct object_data {
      public:
         using object_params = dovah::loaded_forms::structs::region::generable_content::object_params;

      public:
         dovah::form_stub* base_form = nullptr;
         object_params     params;
   };

   class object_collection {
      public:
         using backend_collection_type = dovah::loaded_forms::structs::region::generable_content::raw_object_collection;
         using object_params = object_data::object_params;

         class object {
            public:
               static constexpr const size_t index_of_none = (size_t)-1;

               using child_node_list = std::vector<std::unique_ptr<object>>;

            public:
               object() {}
               object(const object&) = delete;
               object(object&&) noexcept = default;
               object& operator=(const object&) = delete;
               object& operator=(object&&) noexcept = default;

            public:
               object_data     data;
               object*         parent = nullptr;
               child_node_list children;

            public:
               constexpr bool   contains(const object&) const noexcept;
               constexpr size_t index_of(const object&) const noexcept;

               constexpr void clamp_slope_to_ancestor_range(bool descendants_too = true);
               constexpr void set_min_slope(uint8_t);
               constexpr void set_max_slope(uint8_t);
         };

      public:
         object_collection() {}
         object_collection(const object_collection& o) { this->operator=(o); }
         object_collection(object_collection&&) noexcept = default;
         object_collection& operator=(object_collection&&) noexcept = default;

      public:
         std::vector<std::unique_ptr<object>> objects;

         constexpr bool empty() const noexcept;
         constexpr size_t index_of(const object&) const noexcept;

      public:
         void clear();
         void import_data(const dovah::loaded_forms::Region&);
         void import_data(const backend_collection_type&);
         void export_data(dovah::loaded_forms::Region&, backend_collection_type&) const;

         static bool allows_form_type(dovah::form_type);

         object_collection& operator=(const object_collection&);
   };
}
Q_DECLARE_METATYPE(ui::types::regions::generable_content::object_data);

#include "./object_collection.inl"