#include "Region.h"
#include "_common_cpp.h"

#include "../notices/form_load_warnings/by_form_type/region/bad_region_point_list_data_subrecord_size.h"
#include "../notices/form_load_warnings/by_form_type/region/mismatched_region_data_subrecord.h"
#include "../notices/form_load_warnings/by_form_type/region/orphaned_region_data_subrecord.h"
#include "../notices/form_load_warnings/by_form_type/region/region_data_object_has_invalid_parent.h"
#include "../notices/form_load_warnings/by_form_type/region/unknown_region_data_type.h"
#include "../notices/form_load_warnings/by_form_type/region/unused_region_data_subrecord.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::region;
   }
}

namespace dovah::loaded_forms {
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
            (region_data_type)this->region_datas.back().data.index(),
            expected
         );
         intfc.log_load_warning(notice);
      };
      auto _warn_on_unused_region_data = [this, &record, &intfc]() {
         specific_load_warnings::unused_region_data_subrecord notice(
            this->stub,
            record.get_current_subrecord().signature(),
            (region_data_type)this->region_datas.back().data.index()
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
                     auto& area = this->region_areas.emplace_back();
                     subrecord.read(area.edge_falloff);
                  }
                  break;
               case 'RPLD': // Region Point List Data
                  if (this->region_areas.empty())
                     break;
                  if (subrecord.size() % 8) {
                     specific_load_warnings::bad_region_point_list_data_subrecord_size notice(this->stub, subrecord.signature(), subrecord.size());
                     intfc.log_load_warning(notice);
                  }
                  {
                     auto&  area = this->region_areas.back();
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
                        case 2:
                           this->region_datas.emplace_back().data.emplace<2>();
                           break;
                        case 3:
                           this->region_datas.emplace_back().data.emplace<3>();
                           break;
                        case 4:
                           this->region_datas.emplace_back().data.emplace<4>();
                           break;
                        case 5:
                           this->region_datas.emplace_back().data.emplace<5>();
                           break;
                        case 6:
                           this->region_datas.emplace_back().data.emplace<6>();
                           break;
                        case 7:
                           this->region_datas.emplace_back().data.emplace<7>();
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

                     auto& data = this->region_datas.back();
                     subrecord.read(data.override);
                     subrecord.read(data.priority);
                     subrecord.skip_bytes(2);
                  }
                  break;
               #pragma region Types
                  #pragma region Objects
                     case 'RDOB': // legacy
                        if (this->region_datas.empty()) {
                           _warn_on_orphaned_region_data(region_data_type::objects);
                        } else if (auto* data = this->region_datas.back().as_objects()) {
                           uint32_t count = subrecord.size() / 0x18;
                           data->resize(count);
                           for (auto& item : *data) {
                              if (subrecord.is_at_end()) {
                                 item = {};
                                 continue;
                              }
                              subrecord.read(item.form);         // 00
                              subrecord.read(item.parent_index); // 04
                              if (item.parent_index >= 0 && item.parent_index >= data->size()) {
                                 specific_load_warnings::region_data_object_has_invalid_parent notice(
                                    this->stub,
                                    data->size() - 1,
                                    item.parent_index
                                 );
                                 intfc.log_load_warning(notice);
                              }
                              subrecord.skip_bytes(2);
                              {
                                 uint8_t raw = 0;
                                 subrecord.read(raw); // 08
                                 item.density = raw;
                              }
                              subrecord.read(item.clustering);   // 09
                              subrecord.read(item.slope.min);    // 0A
                              subrecord.read(item.slope.max);    // 0B
                              subrecord.read(item.radius_wrt_parent); // 0C
                              subrecord.read(item.radius); // 0E
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
                                       item.height.min = raw;
                                       item.height.max = 2e05;
                                       break;
                                    case max_distance_above_ground:
                                       item.height.min = 0;
                                       item.height.max = raw;
                                       break;
                                    case min_distance_below_ground:
                                       item.height.min = -2e05;
                                       item.height.max = -raw;
                                       break;
                                    case max_distance_below_ground:
                                       item.height.min = -raw;
                                       item.height.max = 0;
                                       break;
                                    case unset:
                                       break;
                                    case max_distance_from_ground:
                                       item.height.min = -raw;
                                       item.height.max = raw;
                                       break;
                                    case anywhere_below_height:
                                       item.height.min = -2e05;
                                       item.height.max = raw;
                                       break;
                                    case anywhere_above_sunken:
                                       item.height.min = -raw;
                                       item.height.max = 2e05;
                                       break;
                                    case anywhere_above_ground:
                                    default:
                                       item.height.min = 0;
                                       item.height.max = 2e05;
                                       break;
                                 }
                              }
                           }
                        } else {
                           _warn_on_mismatched_region_data(region_data_type::objects);
                        }
                        break;
                     case 'RDOT': // modern
                        if (this->region_datas.empty()) {
                           _warn_on_orphaned_region_data(region_data_type::objects);
                        } else if (auto* data = this->region_datas.back().as_objects()) {
                           uint32_t count = subrecord.size() / 0x34;
                           data->resize(count);
                           for (auto& item : *data) {
                              if (subrecord.is_at_end()) {
                                 item = {};
                                 continue;
                              }
                              subrecord.read(item.form);
                              subrecord.read(item.parent_index);
                              if (item.parent_index >= 0 && item.parent_index >= data->size()) {
                                 specific_load_warnings::region_data_object_has_invalid_parent notice(
                                    this->stub,
                                    data->size() - 1,
                                    item.parent_index
                                 );
                                 intfc.log_load_warning(notice);
                              }
                              subrecord.skip_bytes(2);
                              subrecord.read(item.density); // 08
                              subrecord.read(item.clustering); // 0C
                              subrecord.read(item.slope.min); // 0D
                              subrecord.read(item.slope.max); // 0E
                              subrecord.read(item.flags); // 0F
                              subrecord.read(item.radius_wrt_parent); // 10
                              subrecord.read(item.radius); // 12
                              subrecord.read(item.height.min); // 14
                              subrecord.read(item.height.max); // 18
                              subrecord.read(item.sink.base); // 1C
                              subrecord.read(item.sink.variance); // 20
                              subrecord.read(item.angle_variance.x); // 24
                              subrecord.read(item.angle_variance.y); // 26
                              subrecord.read(item.angle_variance.z); // 28
                              subrecord.skip_bytes(2);
                              subrecord.read(item.unk30); // 30
                           }
                        } else {
                           _warn_on_mismatched_region_data(region_data_type::objects);
                        }
                        break;
                  #pragma endregion
                  #pragma region Weather
                     case 'RDWT':
                        if (this->region_datas.empty()) {
                           _warn_on_orphaned_region_data(region_data_type::weather);
                        } else if (auto* data = this->region_datas.back().as_weather()) {
                           auto& item = data->emplace_back();
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
                        if (this->region_datas.empty()) {
                           _warn_on_orphaned_region_data(region_data_type::map);
                        } else if (auto* data = this->region_datas.back().as_map()) {
                           subrecord.read(data->name);
                        } else {
                           _warn_on_mismatched_region_data(region_data_type::map);
                        }
                        break;
                  #pragma endregion
                  #pragma region Landscape
                     case 'ICON':
                        if (this->region_datas.empty()) {
                           _warn_on_orphaned_region_data(region_data_type::landscape);
                        } else if (auto* data = this->region_datas.back().as_landscape()) {
                           subrecord.read(data->texture);
                        } else {
                           _warn_on_mismatched_region_data(region_data_type::landscape);
                        }
                        break;
                     case 'RDLN':
                        if (this->region_datas.empty()) {
                           _warn_on_orphaned_region_data(region_data_type::landscape);
                        } else if (auto* data = this->region_datas.back().as_landscape()) {
                           _warn_on_unused_region_data();
                        } else {
                           _warn_on_mismatched_region_data(region_data_type::landscape);
                        }
                        break;
                  #pragma endregion
                  #pragma region Grass
                     case 'RDGS':
                        if (this->region_datas.empty()) {
                           _warn_on_orphaned_region_data(region_data_type::grass);
                        } else if (auto* data = this->region_datas.back().as_grass()) {
                           uint32_t count = subrecord.size() / 8;
                           data->resize(count);
                           for (auto& item : *data) {
                              if (subrecord.is_at_end()) {
                                 item.object.unmanaged_set(nullptr);
                                 item.parent.unmanaged_set(nullptr);
                                 continue;
                              }
                              if (auto& form = item.object; subrecord.read(form))
                                 intfc.warn_if_ref_is_wrong_type(form, form_type::grass, subrecord.signature());
                              if (auto& form = item.parent; subrecord.read(form))
                                 intfc.warn_if_ref_is_wrong_type(form, form_type::land_texture, subrecord.signature());
                           }
                        } else {
                           _warn_on_mismatched_region_data(region_data_type::grass);
                        }
                        break;
                  #pragma endregion
                  #pragma region Sound
                     case 'RDMO':
                        if (this->region_datas.empty()) {
                           _warn_on_orphaned_region_data(region_data_type::sound);
                        } else if (auto* data = this->region_datas.back().as_sound()) {
                           if (auto& form = data->music; subrecord.read(form))
                              intfc.warn_if_ref_is_wrong_type(form, form_type::music_type, subrecord.signature());
                        } else {
                           _warn_on_mismatched_region_data(region_data_type::sound);
                        }
                        break;
                     case 'RDSA':
                        if (this->region_datas.empty()) {
                           _warn_on_orphaned_region_data(region_data_type::sound);
                        } else if (auto* data = this->region_datas.back().as_sound()) {
                           size_t size = subrecord.size() / 12;
                           data->sounds.resize(size);
                           for (auto& item : data->sounds) {
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
                        if (this->region_datas.empty()) {
                           _warn_on_orphaned_region_data(region_data_type::sound);
                        } else if (auto* data = this->region_datas.back().as_sound()) {
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
      copy->region_areas = this->region_areas;
      {
         auto& src_list = this->region_datas;
         auto& dst_list = copy->region_datas;
         for (auto& data : dst_list) {
            switch (data.data.index()) {
               case 2:
                  for (auto& item : std::get<2>(data.data))
                     item.form.set(*copy, nullptr);
                  break;
               case 3:
                  for (auto& item : std::get<3>(data.data)) {
                     item.weather.set(*copy, nullptr);
                     item.global.set(*copy, nullptr);
                  }
                  break;
               case 6:
                  for (auto& item : std::get<6>(data.data)) {
                     item.object.set(*copy, nullptr);
                     item.parent.set(*copy, nullptr);
                  }
                  break;
               case 7:
                  {
                     auto& casted = std::get<7>(data.data);
                     casted.music.set(*copy, nullptr);
                     for (auto& item : casted.sounds) {
                        item.form.set(*copy, nullptr);
                     }
                  }
                  break;
            }
         }
         dst_list.clear();
         //
         size_t size = src_list.size();
         dst_list.resize(size);
         for (size_t i = 0; i < size; ++i) {
            dst_list[i].override = src_list[i].override;
            dst_list[i].priority = src_list[i].priority;
            switch (src_list[i].data.index()) {
               case 0:
                  dst_list[i].data.emplace<0>();
                  break;
               case 1:
                  dst_list[i].data.emplace<1>();
                  break;
               case 2:
                  {
                     auto& src_data = std::get<2>(src_list[i].data);
                     auto& dst_data = dst_list[i].data.emplace<2>();
                     size_t size = src_data.size();
                     dst_data.resize(size);
                     for (size_t j = 0; j < size; ++j) {
                        auto& src_item = src_data[j];
                        auto& dst_item = dst_data[j];
                        dst_item.form.set(*copy, src_item.form);
                        dst_item.parent_index = src_item.parent_index;
                        dst_item.density = src_item.density;
                        dst_item.clustering = src_item.clustering;
                        dst_item.slope = src_item.slope;
                        dst_item.flags = src_item.flags;
                        dst_item.radius_wrt_parent = src_item.radius_wrt_parent;
                        dst_item.radius = src_item.radius;
                        dst_item.height = src_item.height;
                        dst_item.sink = src_item.sink;
                        dst_item.size_variance = src_item.size_variance;
                        dst_item.angle_variance = src_item.angle_variance;
                        dst_item.unk30 = src_item.unk30;
                     }
                  }
                  break;
               case 3:
                  {
                     auto& src_data = std::get<3>(src_list[i].data);
                     auto& dst_data = dst_list[i].data.emplace<3>();
                     size_t size = src_data.size();
                     dst_data.resize(size);
                     for (size_t j = 0; j < size; ++j) {
                        auto& src_item = src_data[j];
                        auto& dst_item = dst_data[j];
                        dst_item.weather.set(*copy, src_item.weather);
                        dst_item.global.set(*copy, src_item.global);
                        dst_item.chance = src_item.chance;
                     }
                  }
                  break;
               case 4:
                  {
                     auto& src_data = std::get<4>(src_list[i].data);
                     auto& dst_data = dst_list[i].data.emplace<4>();
                     dst_data = src_data;
                  }
                  break;
               case 5:
                  {
                     auto& src_data = std::get<5>(src_list[i].data);
                     auto& dst_data = dst_list[i].data.emplace<5>();
                     dst_data = src_data;
                  }
                  break;
               case 6:
                  {
                     auto& src_data = std::get<6>(src_list[i].data);
                     auto& dst_data = dst_list[i].data.emplace<6>();
                     size_t size = src_data.size();
                     dst_data.resize(size);
                     for (size_t j = 0; j < size; ++j) {
                        auto& src_item = src_data[j];
                        auto& dst_item = dst_data[j];
                        dst_item.object.set(*copy, src_item.object);
                        dst_item.parent.set(*copy, src_item.parent);
                     }
                  }
                  break;
               case 7:
                  {
                     auto& src_data = std::get<7>(src_list[i].data);
                     auto& dst_data = dst_list[i].data.emplace<7>();
                     dst_data.music.set(*copy, src_data.music);
                     size_t size = src_data.sounds.size();
                     dst_data.sounds.resize(size);
                     for (size_t j = 0; j < size; ++j) {
                        auto& src_item = src_data.sounds[j];
                        auto& dst_item = dst_data.sounds[j];
                        dst_item.form.set(*copy, src_item.form);
                        dst_item.flags = src_item.flags;
                        dst_item.chance = src_item.chance;
                     }
                  }
                  break;
            }
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
      record.write_formID_subrecord('WNAM', this->parent_world);
      for (auto& area : this->region_areas) {
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
      for (auto& data : this->region_datas) {
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
            for (auto& item : *casted) {
               subrecord.write(item.form);
               subrecord.write(item.parent_index);
               subrecord.skip_bytes(2);
               subrecord.write(item.density); // 08
               subrecord.write(item.clustering); // 0C
               subrecord.write(item.slope.min); // 0D
               subrecord.write(item.slope.max); // 0E
               subrecord.write(item.flags); // 0F
               subrecord.write(item.radius_wrt_parent); // 10
               subrecord.write(item.radius); // 12
               subrecord.write(item.height.min); // 14
               subrecord.write(item.height.max); // 18
               subrecord.write(item.sink.base); // 1C
               subrecord.write(item.sink.variance); // 20
               subrecord.write(item.angle_variance.x); // 24
               subrecord.write(item.angle_variance.y); // 26
               subrecord.write(item.angle_variance.z); // 28
               subrecord.skip_bytes(2);
               subrecord.write(item.unk30); // 30
            }
            subrecord.close();
         } else if (auto* casted = std::get_if<3>(&data.data)) {
            auto& subrecord = record.open_next_subrecord('RDWT');
            for (auto& item : *casted) {
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
            for (auto& item : *casted) {
               subrecord.write(item.object);
               subrecord.write(item.parent);
            }
            subrecord.close();
         } else if (auto* casted = std::get_if<7>(&data.data)) {
            record.write_formID_subrecord('RDMO', casted->music, true);
            auto& subrecord = record.open_next_subrecord('RDSA');
            for (auto& item : casted->sounds) {
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
      this->region_areas.clear();
      {
         auto& list = this->region_datas;
         for (auto& data : list) {
            switch (data.data.index()) {
               case 2:
                  for (auto& item : std::get<2>(data.data))
                     item.form.set(*this, nullptr);
                  break;
               case 3:
                  for (auto& item : std::get<3>(data.data)) {
                     item.weather.set(*this, nullptr);
                     item.global.set(*this, nullptr);
                  }
                  break;
               case 6:
                  for (auto& item : std::get<6>(data.data)) {
                     item.object.set(*this, nullptr);
                     item.parent.set(*this, nullptr);
                  }
                  break;
               case 7:
                  {
                     auto& casted = std::get<7>(data.data);
                     casted.music.set(*this, nullptr);
                     for (auto& item : casted.sounds) {
                        item.form.set(*this, nullptr);
                     }
                  }
                  break;
            }
         }
         list.clear();
      }
   }
   void Region::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);

      this->parent_world.clear_if(*this, other);
      {
         auto& list = this->region_datas;
         for (auto& data : list) {
            switch (data.data.index()) {
               case 2:
                  for (auto& item : std::get<2>(data.data))
                     item.form.clear_if(*this, other);
                  break;
               case 3:
                  for (auto& item : std::get<3>(data.data)) {
                     item.weather.clear_if(*this, other);
                     item.global.clear_if(*this, other);
                  }
                  break;
               case 6:
                  for (auto& item : std::get<6>(data.data)) {
                     item.object.clear_if(*this, other);
                     item.parent.clear_if(*this, other);
                  }
                  break;
               case 7:
                  {
                     auto& casted = std::get<7>(data.data);
                     casted.music.clear_if(*this, other);
                     for (auto& item : casted.sounds) {
                        item.form.clear_if(*this, other);
                     }
                  }
                  break;
            }
         }
         list.clear();
      }
   }
}