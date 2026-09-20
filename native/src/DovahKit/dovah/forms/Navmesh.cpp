#include "Navmesh.h"
#include "_common_cpp.h"

#include "../notices/form_load_warnings/by_form_type/navmesh/invalid_grid_size.h"
#include "../notices/form_load_warnings/by_form_type/navmesh/too_many_door_links.h"
#include "../notices/form_load_warnings/by_form_type/navmesh/too_many_edge_links.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::navmesh;
   }

   // The CK doesn't save any navmesh-specific data (i.e. only EDID+VMAD are saved) if 
   // the navmesh is flagged as "deleted."
   //
   // There is an edge-case to this behavior: the navmesh is cleared, and its list of 
   // door links is therefore cleared with it; but the doors themselves may still refer 
   // to triangles in this navmesh by virtue of REFR/XNDP. Ideally, whatever's doing the 
   // deleting will properly update the door as well. (DovahKit's backend does.)
   static constexpr bool discard_data_on_save_if_deleted = true;
}

namespace dovah::loaded_forms {
   void Navmesh::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      if (!intfc.is_winning_record)
         return;

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'EDID': // already read by the FormStub
               break;
            case 'VMAD':
               this->script_data.load(subrecord, intfc);
               break;
            case 'OBND':
               //
               // The loader checks for this and passes it to a virtual function on TESForm 
               // that's responsible for loading it. However, this form doesn't derive from 
               // TESBoundObject, so the TESForm implementation of that virtual function (a 
               // no-op) isn't overridden and therefore the data is not retained in memory.
               //
               break;

            case 'NVNM':
               subrecord.read(this->geometry.version);
               this->geometry.pathing_cell.load(subrecord, intfc, &this->stub);
               {
                  auto&    list = this->geometry.vertices;
                  uint32_t size = 0;
                  subrecord.read(size);
                  list.resize(size);
                  for (decltype(size) i = 0; i < size; ++i) {
                     auto& item = list[i];
                     subrecord.read(item.x);
                     subrecord.read(item.y);
                     subrecord.read(item.z);
                  }
               }
               {
                  auto&    list = this->geometry.triangles;
                  uint32_t size = 0;
                  subrecord.read(size);
                  list.resize(size);
                  for (decltype(size) i = 0; i < size; ++i) {
                     auto& item = list[i];
                     for (auto& v : item.vertices)
                        subrecord.read(v);
                     for (auto& v : item.edges)
                        subrecord.read(v);

                     uint32_t combined = 0;
                     subrecord.read(combined);
                     item.flags     = combined;
                     item.cover.raw = combined >> 16;
                  }
               }
               {
                  auto&    list = this->geometry.edge_links;
                  uint32_t size = 0;
                  subrecord.read(size);
                  if (size > std::numeric_limits<uint16_t>::max()) {
                     //
                     // Bethesda serializes the count as four-byte, but uses a two-byte length 
                     // in memory. The CK warns if the serialized count would overflow.
                     //
                     specific_load_warnings::too_many_edge_links notice(
                        this->stub,
                        size
                     );
                     intfc.log_load_warning(notice);
                  }
                  list.resize(size);
                  for (decltype(size) i = 0; i < size; ++i) {
                     auto& item = list[i];
                     subrecord.read(item.type);
                     if (auto& form = item.navmesh; subrecord.read(form))
                        intfc.warn_if_ref_is_wrong_type(form, form_type::navmesh, subrecord.signature());
                     subrecord.read(item.triangle);
                  }
               }
               {
                  auto&    list = this->geometry.door_links;
                  uint32_t size = 0;
                  subrecord.read(size);
                  if (size > std::numeric_limits<uint16_t>::max()) {
                     //
                     // Bethesda serializes the count as four-byte, but uses a two-byte length 
                     // in memory. The CK warns if the serialized count would overflow.
                     //
                     specific_load_warnings::too_many_door_links notice(
                        this->stub,
                        size
                     );
                     intfc.log_load_warning(notice);
                  }
                  list.resize(size);
                  for (decltype(size) i = 0; i < size; ++i) {
                     auto& item = list[i];
                     subrecord.read(item.triangle);
                     subrecord.read(item.crc); // "PathingDoor"
                     if (auto& form = item.door_ref; subrecord.read(form))
                        intfc.warn_if_ref_is_wrong_type(form, form_type::reference, subrecord.signature());
                  }
               }
               {
                  auto&    list = this->geometry.cover_triangles;
                  uint32_t size = 0;
                  subrecord.read(size);
                  list.resize(size);
                  for (auto& item : list)
                     subrecord.read(item);
               }
               {
                  auto& grid = this->geometry.navmesh_grid;
                  subrecord.read(grid.divisor);
                  if (grid.divisor > 12) {
                     specific_load_warnings::invalid_grid_size notice(
                        this->stub,
                        grid.divisor
                     );
                     intfc.log_load_warning(notice);
                  }
                  subrecord.read(grid.size.x);
                  subrecord.read(grid.size.y);
                  subrecord.read(grid.bounds.min.x);
                  subrecord.read(grid.bounds.min.y);
                  subrecord.read(grid.bounds.min.z);
                  subrecord.read(grid.bounds.max.x);
                  subrecord.read(grid.bounds.max.y);
                  subrecord.read(grid.bounds.max.z);
                  if (grid.divisor <= 12) {
                     auto& list = grid.triangles_by_grid_cell;
                     list.resize(grid.divisor * grid.divisor);
                     for (auto& sublist : list) {
                        uint32_t size = 0;
                        subrecord.read(size);
                        sublist.resize(size);
                        for (auto& item : sublist)
                           subrecord.read(item);
                     }
                  }
               }
               break;

            case 'ONAM':
               {
                  uint32_t size = subrecord.size() / 4;
                  this->base_objects.resize(size);
                  for (auto& form : this->base_objects)
                     subrecord.read(form);
               }
               break;
            case 'PNAM':
               {
                  auto& list       = this->preferred_connectors;
                  using value_type = typename std::decay_t<decltype(list)>::value_type;

                  uint32_t size = subrecord.size() / sizeof(value_type);
                  list.resize(size);
                  for (auto& form : list)
                     subrecord.read(form);
               }
               break;
            case 'NNAM':
               {
                  auto& list       = this->non_connectors;
                  using value_type = typename std::decay_t<decltype(list)>::value_type;

                  uint32_t size = subrecord.size() / sizeof(value_type);
                  list.resize(size);
                  for (auto& form : list)
                     subrecord.read(form);
               }
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void Navmesh::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         return;

      struct {
         structs::navmesh_pathing_cell::use_info_state pathing_cell;
         std::vector<form_id_t> door_links;
         std::vector<form_id_t> edge_links;
      } geometry;
      std::vector<form_id_t> base_objects;

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'NVNM':
               subrecord.skip_bytes(sizeof(uint32_t)); // geometry.version
               geometry.pathing_cell.generate_use_info(subrecord);
               {  // vertices
                  uint32_t size = 0;
                  subrecord.read(size);
                  subrecord.skip_bytes(size * sizeof(float) * 3);
               }
               {  // triangles
                  uint32_t size = 0;
                  subrecord.read(size);
                  subrecord.skip_bytes(
                     size * (
                        sizeof(uint16_t) * 3 +
                        sizeof(uint16_t) * 3 +
                        sizeof(uint32_t)
                     )
                  );
               }
               {  // edge links
                  uint32_t size = 0;
                  subrecord.read(size);
                  geometry.edge_links.reserve(size);
                  for (decltype(size) i = 0; i < size; ++i) {
                     subrecord.skip_bytes(sizeof(enum edge_link::type));
                     subrecord.read(geometry.edge_links.emplace_back());
                     subrecord.skip_bytes(sizeof(int16_t));
                  }
               }
               {  // door links
                  uint32_t size = 0;
                  subrecord.read(size);
                  geometry.door_links.reserve(size);
                  for (decltype(size) i = 0; i < size; ++i) {
                     subrecord.skip_bytes(
                        sizeof(door_link::triangle) +
                        sizeof(door_link::crc)
                     );
                     subrecord.read(geometry.door_links.emplace_back());
                  }
               }
               {  // cover triangles
                  uint32_t size = 0;
                  subrecord.read(size);
                  subrecord.skip_bytes(size * sizeof(int16_t));
               }
               {  // navmesh grid
                  uint32_t divisor = 0;
                  subrecord.read(divisor);
                  subrecord.skip_bytes(
                     sizeof(float) * 2 + // size.x, size.y
                     sizeof(float) * 3 + // bounds.min
                     sizeof(float) * 3   // bounds.max
                  );
                  if (divisor <= 12) {
                     subrecord.skip_bytes(
                        (divisor * divisor) * sizeof(int16_t) // triangles_by_grid_cell
                     );
                  }
               }
               break;
            case 'ONAM':
               {
                  uint32_t size = subrecord.size() / 4;
                  base_objects.reserve(size);
                  for (decltype(size) i = 0; i < size; ++i)
                     subrecord.read(base_objects.emplace_back());
               }
               break;
         }
      }
      geometry.pathing_cell.commit_to(uib);
      for (auto id : geometry.door_links)
         uib.add_outbound_reference(id);
      for (auto id : geometry.edge_links)
         uib.add_outbound_reference(id);
      for (auto id : base_objects)
         uib.add_outbound_reference(id);
   }
   void Navmesh::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (Navmesh*)out;

      copy->script_data.clone_from(this->script_data, *copy);

      copy->geometry.version = this->geometry.version;
      copy->geometry.pathing_cell.clone_from(this->geometry.pathing_cell, *copy);
      copy->geometry.vertices = this->geometry.vertices;
      copy->geometry.triangles = this->geometry.triangles;
      {
         auto& src_list = this->geometry.edge_links;
         auto& dst_list = copy->geometry.edge_links;
         for (auto& item : dst_list)
            item.navmesh.set(*copy, nullptr);
         size_t size = src_list.size();
         dst_list.resize(size);
         for (size_t i = 0; i < size; ++i) {
            dst_list[i].navmesh.set(*copy, src_list[i].navmesh);
            dst_list[i].triangle = src_list[i].triangle;
            dst_list[i].type     = src_list[i].type;
         }
      }
      {
         auto& src_list = this->geometry.door_links;
         auto& dst_list = copy->geometry.door_links;
         for (auto& item : dst_list)
            item.door_ref.set(*copy, nullptr);
         size_t size = src_list.size();
         dst_list.resize(size);
         for (size_t i = 0; i < size; ++i) {
            dst_list[i].door_ref.set(*copy, src_list[i].door_ref);
            dst_list[i].triangle = src_list[i].triangle;
            dst_list[i].crc      = src_list[i].crc;
         }
      }
      copy->geometry.cover_triangles = this->geometry.cover_triangles;
      copy->geometry.navmesh_grid = this->geometry.navmesh_grid;

      copy_form_reference_list(*copy, copy->base_objects, this->base_objects);
      copy->preferred_connectors = this->preferred_connectors;
      copy->non_connectors = this->non_connectors;
   }
   void Navmesh::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);

      if constexpr (discard_data_on_save_if_deleted) {
         {  // NVNM
            this->geometry.version = 12;
            this->geometry.pathing_cell.clear(*this);
            this->geometry.vertices.clear();
            this->geometry.triangles.clear();
            {
               auto& list = this->geometry.edge_links;
               for (auto& item : list)
                  item.navmesh.set(*this, nullptr);
               list.clear();
            }
            {
               auto& list = this->geometry.door_links;
               for (auto& item : list)
                  item.door_ref.set(*this, nullptr);
               list.clear();
            }
            this->geometry.cover_triangles.clear();
            this->geometry.navmesh_grid = {};
         }
         clear_form_reference_list(this->base_objects, *this); // ONAM
         this->preferred_connectors.clear(); // PNAM
         this->non_connectors.clear(); // NNAM
         return;
      }

      {
         auto& subrecord = record.open_next_subrecord('NVNM');
         subrecord.reserve_more(
            sizeof(this->geometry.version) +
            8 + // pathing cell
            sizeof(uint32_t) + // vertex count
            this->geometry.vertices.size() * sizeof(float) * 3 +
            sizeof(uint32_t) + // triangle count
            this->geometry.triangles.size() * ((sizeof(uint16_t) * 6) + sizeof(uint32_t)) +
            sizeof(uint32_t) + // edge link count
            this->geometry.edge_links.size() * 10 +
            sizeof(uint32_t) + // door link count
            this->geometry.door_links.size() * 10 +
            sizeof(uint32_t) + // cover triangle count
            this->geometry.cover_triangles.size() * sizeof(uint16_t) +
            (
               sizeof(this->geometry.navmesh_grid.divisor) +
               sizeof(this->geometry.navmesh_grid.size.x) +
               sizeof(this->geometry.navmesh_grid.size.y) +
               sizeof(float) * 6
            )
         );
         subrecord.write(this->geometry.version);
         this->geometry.pathing_cell.save(subrecord, intfc);
         {
            auto& list = this->geometry.vertices;
            subrecord.write((uint32_t)list.size());
            for (auto& item : list) {
               subrecord.write(item.x);
               subrecord.write(item.y);
               subrecord.write(item.z);
            }
         }
         {
            auto& list = this->geometry.triangles;
            subrecord.write((uint32_t)list.size());
            for (auto& item : list) {
               for (auto& v : item.vertices)
                  subrecord.write(v);
               for (auto& v : item.edges)
                  subrecord.write(v);

               uint32_t combined = 0;
               combined |= item.flags;
               combined |= (uint32_t)item.cover.raw << 16;
               subrecord.write(combined);
            }
         }
         {
            auto& list = this->geometry.edge_links;
            subrecord.write((uint32_t)list.size());
            for (auto& item : list) {
               subrecord.write(item.type);
               subrecord.write(item.navmesh);
               subrecord.write(item.triangle);
            }
         }
         {
            auto& list = this->geometry.door_links;
            subrecord.write((uint32_t)list.size());
            for (auto& item : list) {
               subrecord.write(item.triangle);
               subrecord.write(item.crc); // "PathingDoor"
               subrecord.write(item.door_ref);
            }
         }
         {
            auto& list = this->geometry.cover_triangles;
            subrecord.write((uint32_t)list.size());
            for (auto& item : list)
               subrecord.write(item);
         }
         {
            auto& grid = this->geometry.navmesh_grid;
            subrecord.write(grid.divisor);
            subrecord.write(grid.size.x);
            subrecord.write(grid.size.y);
            subrecord.write(grid.bounds.min.x);
            subrecord.write(grid.bounds.min.y);
            subrecord.write(grid.bounds.min.z);
            subrecord.write(grid.bounds.max.x);
            subrecord.write(grid.bounds.max.y);
            subrecord.write(grid.bounds.max.z);
            if (grid.divisor <= 12) {
               auto& list = grid.triangles_by_grid_cell;
               for (auto& sublist : list) {
                  subrecord.reserve_more(sizeof(uint32_t) + sublist.size() * sizeof(uint16_t));
                  subrecord.write((uint32_t)sublist.size());
                  for (auto& item : sublist)
                     subrecord.write(item);
               }
            }
         }
         subrecord.close();
      }

      if (!this->base_objects.empty()) {
         auto& subrecord = record.open_next_subrecord('ONAM');
         subrecord.reserve_more(this->base_objects.size() * 4);
         for (auto& item : this->base_objects)
            subrecord.write(item);
         subrecord.close();
      }
      if (!this->preferred_connectors.empty()) {
         auto& subrecord = record.open_next_subrecord('PNAM');
         subrecord.reserve_more(this->preferred_connectors.size() * sizeof(uint16_t));
         for (auto& item : this->preferred_connectors)
            subrecord.write(item);
         subrecord.close();
      }
      if (!this->non_connectors.empty()) {
         auto& subrecord = record.open_next_subrecord('NNAM');
         subrecord.reserve_more(this->non_connectors.size() * sizeof(uint16_t));
         for (auto& item : this->non_connectors)
            subrecord.write(item);
         subrecord.close();
      }
   }
   void Navmesh::_clear_impl() noexcept {
      this->script_data.clear(*this);

      this->geometry.version = 12;
      this->geometry.pathing_cell.clear(*this);
      this->geometry.vertices.clear();
      this->geometry.triangles.clear();
      {
         auto& list = this->geometry.edge_links;
         for (auto& item : list)
            item.navmesh.set(*this, nullptr);
         list.clear();
      }
      {
         auto& list = this->geometry.door_links;
         for (auto& item : list)
            item.door_ref.set(*this, nullptr);
         list.clear();
      }
      this->geometry.cover_triangles.clear();
      this->geometry.navmesh_grid = {};

      clear_form_reference_list(this->base_objects, *this);
      this->preferred_connectors.clear();
      this->non_connectors.clear();
   }
   void Navmesh::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);

      this->geometry.pathing_cell.sever_outbound_references_to(other, *this);
      {
         auto& list    = this->geometry.edge_links;
         bool  changed = false;
         for (auto& item : list) {
            item.navmesh.clear_if(*this, other);
            if (!item.navmesh)
               changed = true;
         }
         if (changed) {
            std::erase_if(list, [](auto& item) { return !item.navmesh; });
         }
      }
      {
         auto& list    = this->geometry.door_links;
         bool  changed = false;
         for (auto& item : list) {
            item.door_ref.clear_if(*this, other);
            if (!item.door_ref)
               changed = true;
         }
         if (changed) {
            std::erase_if(list, [](auto& item) { return !item.door_ref; });
         }
      }
      remove_form_from_reference_list(this->base_objects, other, *this);
   }
}