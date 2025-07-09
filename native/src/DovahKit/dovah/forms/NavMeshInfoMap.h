#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>
#include "helpers/vector3.h"
#include "Form.h"
#include "_common.h"
#include "./structs/navmesh_info_map/navmesh_info.h"
#include "./structs/navmesh_info_map/navmesh_info_collection.h"
#include "./structs/navmesh_info_map/precomputed_path.h"
#include "./structs/navmesh_info_map/precomputed_path_collection.h"

namespace dovah::loaded_forms {
   class NavMeshInfoMap : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::navmesh_info_map;
         NavMeshInfoMap(const constructor_params& c) : Form(form_type, c) {};

         using navmesh_info            = structs::navmesh_info_map::navmesh_info;
         using navmesh_info_collection = structs::navmesh_info_map::navmesh_info_collection;

         using precomputed_path            = structs::navmesh_info_map::precomputed_path;
         using precomputed_path_collection = structs::navmesh_info_map::precomputed_path_collection;

         static_assert(false, "TODO: These are stored in a map of navmesh info pointers to indices.");
         struct road_marker {
            bool is_active_file_data = false;

            form_reference_t navmesh;
            uint32_t index = 0;
         };

      public:
         navmesh_info_collection       navmesh_infos;     // NVMI[]
         precomputed_path_collection   precomputed_paths; // NVPP
         std::vector<road_marker>      road_markers;      // NVPP
         struct {
            std::vector<form_reference_t> masters;
            std::vector<form_reference_t> active_file;
         } deleted_navmeshes; // NVSI

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
         //
      protected:
         virtual void _clone_impl(Form* out) const noexcept override;
         virtual void _save_impl(tes_file_writing::record& record, load_order_interfaces::form_save& intfc) override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
         virtual void _clear_impl() noexcept override;
   };
}