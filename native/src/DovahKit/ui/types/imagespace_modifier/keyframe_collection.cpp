#include "./keyframe_collection.h"
#include <type_traits>
#include "dovah/forms/ImagespaceModifier.h"

#define FOR_EACH_ANIMATED_PROPERTY(DO) \
   DO(blurs.basic.radius) \
   DO(blurs.motion.strength) \
   DO(blurs.radial.strength) \
   DO(blurs.radial.ramp_up) \
   DO(blurs.radial.start) \
   DO(blurs.radial.ramp_down.start) \
   DO(blurs.radial.ramp_down.value) \
   DO(cinematic.saturation) \
   DO(cinematic.brightness) \
   DO(cinematic.contrast) \
   DO(cinematic.unused) \
   DO(colors.fade) \
   DO(colors.tint) \
   DO(depth_of_field.strength) \
   DO(depth_of_field.distance) \
   DO(depth_of_field.range) \
   DO(double_vision.strength) \
   DO(hdr.bloom.blur_radius) \
   DO(hdr.bloom.scale) \
   DO(hdr.bloom.threshold) \
   DO(hdr.eye_adapt_speed) \
   DO(hdr.target_luminescence.min) \
   DO(hdr.target_luminescence.max) \
   DO(hdr.sky_scale) \
   DO(hdr.sunlight_scale) \
   DO(unknown[0]) \
   DO(unknown[1]) \
   DO(unknown[2]) \
   DO(unknown[3]) \
   DO(unknown[4]) \
   DO(unknown[5]) \
   DO(unknown[6]) \
   DO(unknown[7])

namespace ui::types::imagespace_modifier {
   void keyframe_collection::import_data(const dovah::loaded_forms::ImagespaceModifier& src) {
      this->duration = src.duration;
      this->keyframes.clear();

      using interpolated_color    = dovah::loaded_forms::ImagespaceModifier::interpolated_color;
      using interpolated_float    = dovah::loaded_forms::ImagespaceModifier::interpolated_float;
      using interpolated_mult_add = dovah::loaded_forms::ImagespaceModifier::interpolated_mult_add;

      #define X(prop, ...) \
         [this]<typename PropertyType>(const PropertyType& src_property) -> void { \
            if constexpr (std::is_same_v<PropertyType, interpolated_mult_add>) { \
               for (auto& frame : src_property.mult) { \
                  auto& kf = this->get_or_create_keyframe(frame.time); \
                  kf.prop.mult = frame.value; \
               } \
               for (auto& frame : src_property.add) { \
                  auto& kf = this->get_or_create_keyframe(frame.time); \
                  kf.prop.add  = frame.value; \
               } \
            } else if constexpr (std::is_same_v<PropertyType, interpolated_color>) { \
               for (auto& frame : src_property) { \
                  auto& kf = this->get_or_create_keyframe(frame.time); \
                  kf.prop = QColor(frame.value.r, frame.value.g, frame.value.b, frame.value.unused); \
               } \
            } else { \
               for (auto& frame : src_property) { \
                  auto& kf = this->get_or_create_keyframe(frame.time); \
                  kf.prop = frame.value; \
               } \
            } \
         }(src.prop);
      FOR_EACH_ANIMATED_PROPERTY(X);
      #undef X
   }
   void keyframe_collection::export_data(dovah::loaded_forms::ImagespaceModifier& dst) const {
      dst.duration = this->duration;

      using interpolated_color    = dovah::loaded_forms::ImagespaceModifier::interpolated_color;
      using interpolated_float    = dovah::loaded_forms::ImagespaceModifier::interpolated_float;
      using interpolated_mult_add = dovah::loaded_forms::ImagespaceModifier::interpolated_mult_add;

      #define X(prop, ...) \
         dst.prop = {};
      FOR_EACH_ANIMATED_PROPERTY(X);
      #undef X

      for (const auto& src_keyframe : this->keyframes) {
         #define X(prop, ...) \
            [&src_keyframe]<typename PropertyType>(PropertyType& dst_property) -> void { \
               const auto& src_property = src_keyframe.prop; \
               if constexpr (std::is_same_v<PropertyType, interpolated_mult_add>) { \
                  if (src_property.mult.has_value()) { \
                     auto& dst_frame = dst_property.mult.emplace_back(); \
                     dst_frame.time  = src_keyframe.timestamp; \
                     dst_frame.value = src_property.mult.value(); \
                  } \
                  if (src_property.add.has_value()) { \
                     auto& dst_frame = dst_property.add.emplace_back(); \
                     dst_frame.time  = src_keyframe.timestamp; \
                     dst_frame.value = src_property.add.value(); \
                  } \
               } else if constexpr (std::is_same_v<PropertyType, interpolated_color>) { \
                  if (src_property.has_value()) { \
                     auto& dst_frame   = dst_property.emplace_back(); \
                     dst_frame.time    = src_keyframe.timestamp; \
                     dst_frame.value.r = src_property.value().red(); \
                     dst_frame.value.g = src_property.value().green(); \
                     dst_frame.value.b = src_property.value().blue(); \
                     dst_frame.value.unused = src_property.value().alpha(); \
                  } \
               } else { \
                  if (src_property.has_value()) { \
                     auto& dst_frame = dst_property.emplace_back(); \
                     dst_frame.time  = src_keyframe.timestamp; \
                     dst_frame.value = src_property.value(); \
                  } \
               } \
            }(dst.prop);
         FOR_EACH_ANIMATED_PROPERTY(X);
         #undef X
      }
   }

   keyframe& keyframe_collection::get_or_create_keyframe(float timestamp) {
      if (!this->keyframes.empty()) {
         auto it = std::upper_bound(
            this->keyframes.begin(),
            this->keyframes.end(),
            timestamp,
            [](float desired, const keyframe& item) -> bool {
               return desired <= item.timestamp;
            }
         );
         if (it != this->keyframes.end()) {
            if (it->timestamp == timestamp)
               return *it;
            assert(it->timestamp > timestamp);
            it = this->keyframes.insert(it, {});
            it->timestamp = timestamp;
            return *it;
         }
      }
      auto& kf = this->keyframes.emplace_back();
      kf.timestamp = timestamp;
      return kf;
   }
   computed_keyframe keyframe_collection::get_computed_keyframe(float timestamp) const noexcept {
      computed_keyframe interpolated;
      auto _interpolate_property = [this, timestamp, &interpolated](auto& dst_property, auto&& src_property_getter) {
         using dst_property_type = std::decay_t<decltype(dst_property)>;

         auto _interpolate_list = [this, timestamp](auto& dst_subvalue, auto&& src_subvalue_list_getter) {
            using dst_subvalue_type = std::decay_t<decltype(dst_subvalue)>;

            struct found {
               float time = 0;
               dst_subvalue_type value = {};
            };
            std::optional<found> found_prev;
            std::optional<found> found_next;
            for (const auto& src_keyframe : this->keyframes) {
               const auto& src_value_opt = src_subvalue_list_getter(src_keyframe);
               if (!src_value_opt.has_value())
                  continue;
               const auto  src_value = src_value_opt.value();
               auto src_time = src_keyframe.timestamp;
               if (src_time <= timestamp) {
                  if (!found_prev.has_value() || found_prev.value().time < src_time)
                     found_prev = found{ src_time, src_value };
               }
               if (src_time <= timestamp) {
                  if (!found_next.has_value() || found_next.value().time > src_time)
                     found_next = found{ src_time, src_value };
               }
            }
            if (found_prev.has_value() && found_next.has_value()) {
               const auto& a = found_prev.value();
               const auto& b = found_next.value();
               if (a.time == b.time) {
                  dst_subvalue = a.value;
                  return;
               }
               float timespan = b.time - a.time;
               float factor = (timestamp - found_prev.value().time) / timespan;
               float inv_factor = 1.0F - factor;
               if constexpr (std::is_same_v<dst_subvalue_type, float>) {
                  dst_subvalue = (b.value * factor) + (a.value * inv_factor);
               } else if constexpr (std::is_same_v<dst_subvalue_type, QColor>) {
                  dst_subvalue.setRedF((b.value.redF() * factor) + (a.value.redF() * inv_factor));
                  dst_subvalue.setGreenF((b.value.greenF() * factor) + (a.value.greenF() * inv_factor));
                  dst_subvalue.setBlueF((b.value.blueF() * factor) + (a.value.blueF() * inv_factor));
                  dst_subvalue.setAlphaF((b.value.alphaF() * factor) + (a.value.alphaF() * inv_factor));
               }
            }
            if (!found_prev.has_value() && !found_next.has_value())
               return;
            if (found_prev.has_value())
               dst_subvalue = found_prev.value().value;
            else if (found_next.has_value())
               dst_subvalue = found_next.value().value;

         };

         if constexpr (std::is_same_v<dst_property_type, computed_keyframe::mult_add>) {
            _interpolate_list(dst_property.mult, [&src_property_getter](const keyframe& src_keyframe) { return src_property_getter(src_keyframe).mult; });
            _interpolate_list(dst_property.add,  [&src_property_getter](const keyframe& src_keyframe) { return src_property_getter(src_keyframe).add; });
         } else {
            _interpolate_list(dst_property, src_property_getter);
         }
      };
      
      #define X(prop, ...) \
         _interpolate_property(interpolated.prop, [this](const keyframe& kf) -> auto& { return kf.prop; });
      FOR_EACH_ANIMATED_PROPERTY(X);
      #undef X
      
      return interpolated;
   }

   void keyframe_collection::strip_empty_keyframes() {
      auto& list = this->keyframes;
      std::erase_if(
         list,
         [](const auto& kf) -> bool {
            //
            // Lambda needed or we'll choke on if-constexpr-false branches that 
            // access members not present on the current field.
            //
            #define X(prop, ...) \
               bool exit = [](auto& property) -> bool { \
                  using property_type = std::decay_t<decltype(property)>; \
                  if constexpr (std::is_same_v<property_type, keyframe::mult_add>) { \
                     return property.mult.has_value() || property.add.has_value(); \
                  } else { \
                     return property.has_value(); \
                  } \
               }(kf.prop); \
               if (!exit) \
                  return true;
            FOR_EACH_ANIMATED_PROPERTY(X);
            #undef X
            return true;
         }
      );
   }
}