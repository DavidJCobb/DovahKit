#include "Region.h"
#include "_common_cpp.h"

#include "../notices/form_load_warnings/by_form_type/region/bad_region_point_list_data_subrecord_size.h"
#include "../notices/form_load_warnings/by_form_type/region/invalid_areas.h"
#include "../notices/form_load_warnings/by_form_type/region/mismatched_region_data_subrecord.h"
#include "../notices/form_load_warnings/by_form_type/region/multiple_data_collections_of_same_type.h"
#include "../notices/form_load_warnings/by_form_type/region/orphaned_region_data_subrecord.h"
#include "../notices/form_load_warnings/by_form_type/region/region_data_object_has_invalid_parent.h"
#include "../notices/form_load_warnings/by_form_type/region/unknown_region_data_type.h"
#include "../notices/form_load_warnings/by_form_type/region/unused_region_data_subrecord.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::region;
   }
   namespace generable_content {
      using namespace dovah::loaded_forms::structs::region::generable_content;
   }
}

namespace dovah::loaded_forms {
   void Region::delete_invalid_areas() {
      auto&  list = this->areas;
      size_t size = list.size();
      for (size_t i = 0; i < size; ++i) {
         auto& area = list[i];
         if (!area.is_valid(true)) {
            list.erase(list.begin() + i);
            --i;
            --size;
         }
      }
   }

   void Region::fold_generable_content() {
      auto&  list = this->generable_content;
      size_t size = list.size();

      constexpr const size_t index_of_none = (size_t)-1;

      {  // Objects
         using collection_type = structs::region::generable_content::raw_object_collection;

         size_t fold_into_index = index_of_none;
         for (size_t i = 0; i < size; ++i) {
            auto& item = list[i];
            auto* here = item.as<collection_type>();
            if (!here)
               continue;
            if (fold_into_index == index_of_none) {
               fold_into_index = i;
               continue;
            }
            //
            // Merge them.
            //
            list[fold_into_index].as<collection_type>()->copy_insert_from(*this, *here);
            here->clear(*this);
            list.erase(list.begin() + i);
            --i;
            --size;
         }
      }
      {  // Weather
         using collection_type = structs::region::generable_content::weather_collection;

         size_t fold_into_index = index_of_none;
         for (size_t i = 0; i < size; ++i) {
            auto& item = list[i];
            auto* here = item.as<collection_type>();
            if (!here)
               continue;
            if (fold_into_index == index_of_none) {
               fold_into_index = i;
               continue;
            }
            //
            // Merge them.
            //
            list[fold_into_index].as<collection_type>()->copy_insert_from(*this, *here);
            here->clear(*this);
            list.erase(list.begin() + i);
            --i;
            --size;
         }
      }
      {  // Map
         using collection_type = structs::region::generable_content::map;

         size_t fold_into_index = index_of_none;
         for (size_t i = 0; i < size; ++i) {
            auto& item = list[i];
            auto* here = item.as<collection_type>();
            if (!here)
               continue;
            if (fold_into_index == index_of_none) {
               fold_into_index = i;
               continue;
            }
            //
            // Merge them.
            //
            if (!here->name.empty()) {
               list[fold_into_index].as<collection_type>()->name = here->name;
            }
            list.erase(list.begin() + i);
            --i;
            --size;
         }
      }
      {  // Landscape
         using collection_type = structs::region::generable_content::landscape;

         size_t fold_into_index = index_of_none;
         for (size_t i = 0; i < size; ++i) {
            auto& item = list[i];
            auto* here = item.as<collection_type>();
            if (!here)
               continue;
            if (fold_into_index == index_of_none) {
               fold_into_index = i;
               continue;
            }
            //
            // Merge them.
            //
            if (!here->texture.empty()) {
               list[fold_into_index].as<collection_type>()->texture = here->texture;
            }
            list.erase(list.begin() + i);
            --i;
            --size;
         }
      }
      {  // Grass
         using collection_type = structs::region::generable_content::grass_collection;

         size_t fold_into_index = index_of_none;
         for (size_t i = 0; i < size; ++i) {
            auto& item = list[i];
            auto* here = item.as<collection_type>();
            if (!here)
               continue;
            if (fold_into_index == index_of_none) {
               fold_into_index = i;
               continue;
            }
            //
            // Merge them.
            //
            list[fold_into_index].as<collection_type>()->copy_insert_from(*this, *here);
            here->clear(*this);
            list.erase(list.begin() + i);
            --i;
            --size;
         }
      }
      {  // Audio
         using collection_type = structs::region::generable_content::audio;

         size_t fold_into_index = index_of_none;
         for (size_t i = 0; i < size; ++i) {
            auto& item = list[i];
            auto* here = item.as<collection_type>();
            if (!here)
               continue;
            if (fold_into_index == index_of_none) {
               fold_into_index = i;
               continue;
            }
            //
            // Merge them.
            //
            list[fold_into_index].as<collection_type>()->copy_insert_from(*this, *here);
            here->clear(*this);
            list.erase(list.begin() + i);
            --i;
            --size;
         }
      }
   }

   void Region::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      if (!intfc.is_winning_record)
         return;

      auto _warn_on_orphaned_region_data = [this, &record, &intfc](region_data_type expected) {
         specific_load_warnings::orphaned_region_data_subrecord notice(
            this->stub,
            record.get_current_subrecord().signature(),
            expected
         );
         intfc.log_load_warning(notice);
      };
      auto _warn_on_mismatched_region_data = [this, &record, &intfc](region_data_type expected) {
         specific_load_warnings::mismatched_region_data_subrecord notice(
            this->stub,
            record.get_current_subrecord().signature(),
            (region_data_type)this->generable_content.back().data.index(),
            expected
         );
         intfc.log_load_warning(notice);
      };
      auto _warn_on_unused_region_data = [this, &record, &intfc]() {
         specific_load_warnings::unused_region_data_subrecord notice(
            this->stub,
            record.get_current_subrecord().signature(),
            (region_data_type)this->generable_content.back().data.index()
         );
         intfc.log_load_warning(notice);
      };

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'EDID': // already read by the FormStub
               break;
            case 'VMAD':
               this->script_data.load(subrecord, intfc);
               break;

            case 'RCLR':
               this->map_color.load(subrecord);
               break;
            case 'WNAM':
               if (auto& form = this->parent_world; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::worldspace, subrecord.signature());
               break;

            #pragma region Region Areas
               case 'RPLI': // Region Point LIst
                  {
                     auto& area = this->areas.emplace_back();
                     subrecord.read(area.edge_falloff);
                  }
                  break;
               case 'RPLD': // Region Point List Data
                  if (this->areas.empty())
                     break;
                  if (subrecord.size() % 8) {
                     specific_load_warnings::bad_region_point_list_data_subrecord_size notice(this->stub, subrecord.signature(), subrecord.size());
                     intfc.log_load_warning(notice);
                  }
                  {
                     auto&  area = this->areas.back();
                     size_t size = subrecord.size() / 8;
                     area.points.resize(size);
                     for (auto& point : area.points) {
                        point = {};
                        subrecord.read(point.x);
                        subrecord.read(point.y);
                     }
                  }
                  break;
            #pragma endregion

            #pragma region Region Data
               case 'RDAT':
                  {
                     uint32_t type = 0;
                     subrecord.read(type);
                     bool valid = true;
                     switch (type) {
                        case 0:
                           this->generable_content.emplace_back().data.emplace<0>();
                           break;
                        case 1:
                           this->generable_content.emplace_back().data.emplace<1>();
                           break;
                        case 2:
                           this->generable_content.emplace_back().data.emplace<2>();
                           break;
                        case 3:
                           this->generable_content.emplace_back().data.emplace<3>();
                           break;
                        case 4:
                           this->generable_content.emplace_back().data.emplace<4>();
                           break;
                        case 5:
                           this->generable_content.emplace_back().data.emplace<5>();
                           break;
                        case 6:
                           this->generable_content.emplace_back().data.emplace<6>();
                           break;
                        case 7:
                           this->generable_content.emplace_back().data.emplace<7>();
                           break;
                        default:
                           {
                              specific_load_warnings::unknown_region_data_type notice(this->stub, type);
                              intfc.log_load_warning(notice);
                           }
                           valid = false;
                           break;
                     }
                     if (!valid)
                        break;

                     auto& data = this->generable_content.back();
                     subrecord.read(data.override);
                     subrecord.read(data.priority);
                     subrecord.skip_bytes(2);
                  }
                  break;
               #pragma region Types
                  #pragma region Objects
                     case 'RDOB': // legacy
                        if (this->generable_content.empty()) {
                           _warn_on_orphaned_region_data(region_data_type::objects);
                        } else if (auto* data = this->generable_content.back().as<generable_content::raw_object_collection>()) {
                           uint32_t count = subrecord.size() / 0x18;
                           data->objects.resize(count);
                           for (auto& item : data->objects) {
                              if (subrecord.is_at_end()) {
                                 item = {};
                                 continue;
                              }
                              subrecord.read(item.form);         // 00
                              subrecord.read(item.parent_index); // 04
                              if (item.parent_index >= 0 && item.parent_index >= count) {
                                 specific_load_warnings::region_data_object_has_invalid_parent notice(
                                    this->stub,
                                    count - 1,
                                    item.parent_index
                                 );
                                 intfc.log_load_warning(notice);
                              }
                              subrecord.skip_bytes(2);
                              {
                                 uint8_t raw = 0;
                                 subrecord.read(raw); // 08
                                 item.params.density = raw;
                              }
                              subrecord.read(item.params.clustering);   // 09
                              subrecord.read(item.params.slope.min);    // 0A
                              subrecord.read(item.params.slope.max);    // 0B
                              subrecord.read(item.params.radius_wrt_parent); // 0C
                              subrecord.read(item.params.radius); // 0E
                              {
                                 enum class height_type : uint32_t {
                                    min_distance_above_ground = 0, // [      x, 200000]
                                    max_distance_above_ground = 1, // [      0,      x]
                                    min_distance_below_ground = 2, // [-200000,     -x]
                                    max_distance_below_ground = 3, // [     -x,      0]
                                    unset                     = 4,
                                    max_distance_from_ground  = 5, // [     -x,      x]
                                    anywhere_below_height     = 6, // [-200000,      x]
                                    anywhere_above_sunken     = 7, // [     -x, 200000]
                                    anywhere_above_ground     = 8, // [      0, 200000]
                                 };

                                 uint16_t raw  = 0;
                                 subrecord.read(raw); // 10
                                 subrecord.skip_bytes(2);
                                 height_type type = (height_type)0;
                                 subrecord.read(type); // 14
                                 switch (type) {
                                    using enum height_type;
                                    case min_distance_above_ground:
                                       item.params.height.min = raw;
                                       item.params.height.max = 2e05;
                                       break;
                                    case max_distance_above_ground:
                                       item.params.height.min = 0;
                                       item.params.height.max = raw;
                                       break;
                                    case min_distance_below_ground:
                                       item.params.height.min = -2e05;
                                       item.params.height.max = -raw;
                                       break;
                                    case max_distance_below_ground:
                                       item.params.height.min = -raw;
                                       item.params.height.max = 0;
                                       break;
                                    case unset:
                                       break;
                                    case max_distance_from_ground:
                                       item.params.height.min = -raw;
                                       item.params.height.max = raw;
                                       break;
                                    case anywhere_below_height:
                                       item.params.height.min = -2e05;
                                       item.params.height.max = raw;
                                       break;
                                    case anywhere_above_sunken:
                                       item.params.height.min = -raw;
                                       item.params.height.max = 2e05;
                                       break;
                                    case anywhere_above_ground:
                                    default:
                                       item.params.height.min = 0;
                                       item.params.height.max = 2e05;
                                       break;
                                 }
                              }
                           }
                        } else {
                           _warn_on_mismatched_region_data(region_data_type::objects);
                        }
                        break;
                     case 'RDOT': // modern
                        if (this->generable_content.empty()) {
                           _warn_on_orphaned_region_data(region_data_type::objects);
                        } else if (auto* data = this->generable_content.back().as<generable_content::raw_object_collection>()) {
                           uint32_t count = subrecord.size() / 0x34;
                           data->objects.resize(count);
                           for (auto& item : data->objects) {
                              if (subrecord.is_at_end()) {
                                 item = {};
                                 continue;
                              }
                              subrecord.read(item.form);
                              subrecord.read(item.parent_index);
                              if (item.parent_index >= 0 && item.parent_index >= count) {
                                 specific_load_warnings::region_data_object_has_invalid_parent notice(
                                    this->stub,
                                    count - 1,
                                    item.parent_index
                                 );
                                 intfc.log_load_warning(notice);
                              }
                              subrecord.skip_bytes(2);
                              subrecord.read(item.params.density); // 08
                              subrecord.read(item.params.clustering); // 0C
                              subrecord.read(item.params.slope.min); // 0D
                              subrecord.read(item.params.slope.max); // 0E
                              subrecord.read(item.params.flags); // 0F
                              subrecord.read(item.params.radius_wrt_parent); // 10
                              subrecord.read(item.params.radius); // 12
                              subrecord.read(item.params.height.min); // 14
                              subrecord.read(item.params.height.max); // 18
                              subrecord.read(item.params.sink.base); // 1C
                              subrecord.read(item.params.sink.variance); // 20
                              subrecord.read(item.params.angle_variance.x); // 24
                              subrecord.read(item.params.angle_variance.y); // 26
                              subrecord.read(item.params.angle_variance.z); // 28
                              subrecord.read(item.params.unk2E); // 2E
                              subrecord.read(item.params.paint_vertices.color.r); // 30
                              subrecord.read(item.params.paint_vertices.color.g); // 31
                              subrecord.read(item.params.paint_vertices.color.b); // 32
                              subrecord.read(item.params.paint_vertices.radius_percent); // 33
                           }
                        } else {
                           _warn_on_mismatched_region_data(region_data_type::objects);
                        }
                        break;
                  #pragma endregion
                  #pragma region Weather
                     case 'RDWT':
                        if (this->generable_content.empty()) {
                           _warn_on_orphaned_region_data(region_data_type::weather);
                        } else if (auto* data = this->generable_content.back().as<generable_content::weather_collection>()) {
                           auto& item = data->weathers.emplace_back();
                           if (auto& form = item.weather; subrecord.read(form))
                              intfc.warn_if_ref_is_wrong_type(form, form_type::weather, subrecord.signature());
                           subrecord.read(item.chance);
                           if (auto& form = item.global; subrecord.read(form))
                              intfc.warn_if_ref_is_wrong_type(form, form_type::global, subrecord.signature());
                        } else {
                           _warn_on_mismatched_region_data(region_data_type::weather);
                        }
                        break;
                  #pragma endregion
                  #pragma region Map
                     case 'RDMP':
                        if (this->generable_content.empty()) {
                           _warn_on_orphaned_region_data(region_data_type::map);
                        } else if (auto* data = this->generable_content.back().as<generable_content::map>()) {
                           subrecord.read(data->name);
                        } else {
                           _warn_on_mismatched_region_data(region_data_type::map);
                        }
                        break;
                  #pragma endregion
                  #pragma region Landscape
                     case 'ICON':
                        if (this->generable_content.empty()) {
                           _warn_on_orphaned_region_data(region_data_type::landscape);
                        } else if (auto* data = this->generable_content.back().as<generable_content::landscape>()) {
                           subrecord.read(data->texture);
                        } else {
                           _warn_on_mismatched_region_data(region_data_type::landscape);
                        }
                        break;
                     case 'RDLN':
                        if (this->generable_content.empty()) {
                           _warn_on_orphaned_region_data(region_data_type::landscape);
                        } else if (auto* data = this->generable_content.back().as<generable_content::landscape>()) {
                           _warn_on_unused_region_data();
                        } else {
                           _warn_on_mismatched_region_data(region_data_type::landscape);
                        }
                        break;
                  #pragma endregion
                  #pragma region Grass
                     case 'RDGS':
                        if (this->generable_content.empty()) {
                           _warn_on_orphaned_region_data(region_data_type::grass);
                        } else if (auto* data = this->generable_content.back().as<generable_content::grass_collection>()) {
                           uint32_t count = subrecord.size() / 8;
                           data->entries.resize(count);
                           for (auto& item : data->entries) {
                              if (subrecord.is_at_end()) {
                                 item.grass.unmanaged_set(nullptr);
                                 item.land_texture.unmanaged_set(nullptr);
                                 continue;
                              }
                              if (auto& form = item.grass; subrecord.read(form))
                                 intfc.warn_if_ref_is_wrong_type(form, form_type::grass, subrecord.signature());
                              if (auto& form = item.land_texture; subrecord.read(form))
                                 intfc.warn_if_ref_is_wrong_type(form, form_type::land_texture, subrecord.signature());
                           }
                        } else {
                           _warn_on_mismatched_region_data(region_data_type::grass);
                        }
                        break;
                  #pragma endregion
                  #pragma region Sound
                     case 'RDMO':
                        if (this->generable_content.empty()) {
                           _warn_on_orphaned_region_data(region_data_type::sound);
                        } else if (auto* data = this->generable_content.back().as<generable_content::audio>()) {
                           if (auto& form = data->music; subrecord.read(form))
                              intfc.warn_if_ref_is_wrong_type(form, form_type::music_type, subrecord.signature());
                        } else {
                           _warn_on_mismatched_region_data(region_data_type::sound);
                        }
                        break;
                     case 'RDSA':
                        if (this->generable_content.empty()) {
                           _warn_on_orphaned_region_data(region_data_type::sound);
                        } else if (auto* data = this->generable_content.back().as<generable_content::audio>()) {
                           size_t size = subrecord.size() / 12;
                           data->ambient_sounds.resize(size);
                           for (auto& item : data->ambient_sounds) {
                              item.form.unmanaged_set(nullptr);
                              item.chance = 0;
                              item.flags  = 0;

                              if (auto& form = item.form; subrecord.read(form))
                                 intfc.warn_if_ref_is_wrong_type(form, form_type::sound_descriptor, subrecord.signature());
                              subrecord.read(item.flags);
                              subrecord.read(item.chance);
                           }
                        } else {
                           _warn_on_mismatched_region_data(region_data_type::sound);
                        }
                        break;
                     case 'RDSD':
                     case 'RDMD':
                        if (this->generable_content.empty()) {
                           _warn_on_orphaned_region_data(region_data_type::sound);
                        } else if (auto* data = this->generable_content.back().as<generable_content::audio>()) {
                           _warn_on_unused_region_data();
                        } else {
                           _warn_on_mismatched_region_data(region_data_type::sound);
                        }
                        break;
                  #pragma endregion
               #pragma endregion
            #pragma endregion

            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }

      size_t invalid_area_count = 0;
      for (auto& area : this->areas) {
         if (!area.is_valid(true))
            ++invalid_area_count;
      }
      if (invalid_area_count > 0) {
         specific_load_warnings::invalid_areas notice(
            this->stub,
            invalid_area_count
         );
         intfc.log_load_warning(notice);
      }

      std::array<size_t, (size_t)(region_data_type::sound) + 1> count_by_type = {};
      for (auto& data : this->generable_content) {
         auto opt = data.type();
         if (!opt.has_value())
            continue;
         auto type = opt.value();
         if ((size_t)type < count_by_type.size()) {
            ++count_by_type[(size_t)type];
         }
      }
      for (size_t i = 0; i < count_by_type.size(); ++i) {
         auto count = count_by_type[i];
         auto type  = (region_data_type)i;
         if (count > 1) {
            specific_load_warnings::multiple_data_collections_of_same_type notice(this->stub, type, count);
            intfc.log_load_warning(notice);
         }
      }
   }
   /*static*/ void Region::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files. (TODO: CONFIRM THIS)
         //
         return;

      form_id_t parent_world;

      region_data_type last_seen_type = (region_data_type)-1;
      std::vector<form_id_t> region_data;
      form_id_t region_data_music;

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;

            case 'WNAM':
               subrecord.read(parent_world);
               break;
               
            #pragma region Region Data
               case 'RDAT':
                  for (auto id : region_data)
                     uib.add_outbound_reference(id);
                  if (region_data_music)
                     uib.add_outbound_reference(region_data_music);

                  region_data.clear();
                  region_data_music = {};
                  //
                  subrecord.read(last_seen_type);
                  break;
               #pragma region Types
                  #pragma region Objects
                     case 'RDOB':
                        if (last_seen_type == region_data_type::objects) {
                           uint32_t count = subrecord.size() / 0x34;
                           region_data.clear();
                           region_data.resize(count);
                           for (auto& id : region_data) {
                              subrecord.read(id);
                              subrecord.skip_bytes(0x14);
                           }
                        }
                        break;
                     case 'RDOT':
                        if (last_seen_type == region_data_type::objects) {
                           uint32_t count = subrecord.size() / 0x34;
                           region_data.clear();
                           region_data.resize(count);
                           for (auto& id : region_data) {
                              subrecord.read(id);
                              subrecord.skip_bytes(0x30);
                           }
                        }
                        break;
                  #pragma endregion
                  #pragma region Weather
                     case 'RDWT':
                        if (last_seen_type == region_data_type::weather) {
                           uint32_t count = subrecord.size() / 12;
                           region_data.clear();
                           region_data.resize(count * 2);
                           for (uint32_t i = 0; i < count; ++i) {
                              subrecord.read(region_data[i * 2 + 0]); // weather
                              subrecord.skip_bytes(4);
                              subrecord.read(region_data[i * 2 + 1]); // global
                           }
                        }
                        break;
                  #pragma endregion
                  #pragma region Map
                     case 'RDMP':
                        break;
                  #pragma endregion
                  #pragma region Landscape
                     case 'ICON':
                     case 'RDLN':
                        break;
                  #pragma endregion
                  #pragma region Grass
                     case 'RDGS':
                        if (last_seen_type == region_data_type::grass) {
                           uint32_t count = subrecord.size() / 8;
                           region_data.clear();
                           region_data.resize(count * 2);
                           for (uint32_t i = 0; i < count; ++i) {
                              subrecord.read(region_data[i * 2 + 0]); // grass
                              subrecord.read(region_data[i * 2 + 1]); // land texture
                           }
                        }
                        break;
                  #pragma endregion
                  #pragma region Sound
                     case 'RDMO':
                        if (last_seen_type == region_data_type::sound) {
                           subrecord.read(region_data_music);
                        }
                        break;
                     case 'RDSA':
                        if (last_seen_type == region_data_type::weather) {
                           uint32_t count = subrecord.size() / 12;
                           region_data.clear();
                           region_data.resize(count);
                           for (uint32_t i = 0; i < count; ++i) {
                              subrecord.read(region_data[i]);
                              subrecord.skip_bytes(8);
                           }
                        }
                        break;
                     case 'RDSD':
                     case 'RDMD':
                        break;
                  #pragma endregion
               #pragma endregion
            #pragma endregion
         }
      }
      uib.add_outbound_reference(parent_world);
      //
      for (auto id : region_data)
         uib.add_outbound_reference(id);
      if (region_data_music)
         uib.add_outbound_reference(region_data_music);
   }
   void Region::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (Region*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);

      copy->map_color = this->map_color;
      copy->parent_world.set(*copy, this->parent_world);
      copy->areas = this->areas;
      {
         auto& src_list = this->generable_content;
         auto& dst_list = copy->generable_content;
         for (auto& dst_item : dst_list)
            dst_item.clear(*copy);
         dst_list.clear();
         
         size_t size = src_list.size();
         dst_list.resize(size);
         for (size_t i = 0; i < size; ++i) {
            dst_list[i].clone_from(*copy, src_list[i]);
         }
      }
   }
   void Region::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      {
         auto& subrecord = record.open_next_subrecord('RCLR');
         this->map_color.save(subrecord);
         subrecord.close();
      }
      record.write_formID_subrecord('WNAM', this->parent_world, true);
      for (auto& area : this->areas) {
         {
            auto& subrecord = record.open_next_subrecord('RPLI');
            subrecord.write(area.edge_falloff);
            subrecord.close();
         }
         {
            auto& subrecord = record.open_next_subrecord('RPLD');
            for (auto& point : area.points) {
               subrecord.write(point.x);
               subrecord.write(point.y);
            }
            subrecord.close();
         }
      }
      for (auto& data : this->generable_content) {
         {
            auto& subrecord = record.open_next_subrecord('RDAT');
            subrecord.write((uint32_t)data.data.index());
            subrecord.write(data.override);
            subrecord.write(data.priority);
            subrecord.skip_bytes(2);
            subrecord.close();
         }
         if (auto* casted = std::get_if<2>(&data.data)) {
            auto& subrecord = record.open_next_subrecord('RDOT');
            for (auto& item : casted->objects) {
               subrecord.write(item.form);
               subrecord.write(item.parent_index);
               subrecord.skip_bytes(2);
               subrecord.write(item.params.density); // 08
               subrecord.write(item.params.clustering); // 0C
               subrecord.write(item.params.slope.min); // 0D
               subrecord.write(item.params.slope.max); // 0E
               subrecord.write(item.params.flags); // 0F
               subrecord.write(item.params.radius_wrt_parent); // 10
               subrecord.write(item.params.radius); // 12
               subrecord.write(item.params.height.min); // 14
               subrecord.write(item.params.height.max); // 18
               subrecord.write(item.params.sink.base); // 1C
               subrecord.write(item.params.sink.variance); // 20
               subrecord.write(item.params.angle_variance.x); // 24
               subrecord.write(item.params.angle_variance.y); // 26
               subrecord.write(item.params.angle_variance.z); // 28
               subrecord.write(item.params.unk2E); // 2E
               subrecord.write(item.params.paint_vertices.color.r); // 30
               subrecord.write(item.params.paint_vertices.color.g); // 31
               subrecord.write(item.params.paint_vertices.color.b); // 32
               subrecord.write(item.params.paint_vertices.radius_percent); // 33
            }
            subrecord.close();
         } else if (auto* casted = std::get_if<3>(&data.data)) {
            auto& subrecord = record.open_next_subrecord('RDWT');
            for (auto& item : casted->weathers) {
               subrecord.write(item.weather);
               subrecord.write(item.chance);
               subrecord.write(item.global);
            }
            subrecord.close();
         } else if (auto* casted = std::get_if<4>(&data.data)) {
            auto& subrecord = record.open_next_subrecord('RDMP');
            subrecord.write(casted->name);
            subrecord.close();
         } else if (auto* casted = std::get_if<5>(&data.data)) {
            auto& subrecord = record.open_next_subrecord('ICON');
            subrecord.write(casted->texture);
            subrecord.close();
         } else if (auto* casted = std::get_if<6>(&data.data)) {
            auto& subrecord = record.open_next_subrecord('RDGS');
            for (auto& item : casted->entries) {
               subrecord.write(item.grass);
               subrecord.write(item.land_texture);
            }
            subrecord.close();
         } else if (auto* casted = std::get_if<7>(&data.data)) {
            record.write_formID_subrecord('RDMO', casted->music, true);
            auto& subrecord = record.open_next_subrecord('RDSA');
            for (auto& item : casted->ambient_sounds) {
               subrecord.write(item.form);
               subrecord.write(item.flags);
               subrecord.write(item.chance);
            }
            subrecord.close();
         }
      }
   }
   void Region::_clear_impl() noexcept {
      this->script_data.clear(*this);

      this->map_color = {};
      this->parent_world.set(*this, nullptr);
      this->areas.clear();
      {
         auto& list = this->generable_content;
         for (auto& item : list)
            item.clear(*this);
         list.clear();
      }
   }
   void Region::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);

      this->parent_world.clear_if(*this, other);
      {
         auto& list = this->generable_content;
         for (auto& item : list)
            item.sever_references_to(*this, other);
         list.clear();
      }
   }
}