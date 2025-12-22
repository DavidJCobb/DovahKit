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
   DO(cinematic.saturation.mult) \
   DO(cinematic.saturation.add) \
   DO(cinematic.brightness.mult) \
   DO(cinematic.brightness.add) \
   DO(cinematic.contrast.mult) \
   DO(cinematic.contrast.add) \
   DO(cinematic.unused.mult) \
   DO(cinematic.unused.add) \
   DO(colors.fade) \
   DO(colors.tint) \
   DO(depth_of_field.strength) \
   DO(depth_of_field.distance) \
   DO(depth_of_field.range) \
   DO(double_vision.strength) \
   DO(hdr.bloom.blur_radius.mult) \
   DO(hdr.bloom.blur_radius.add) \
   DO(hdr.bloom.scale.mult) \
   DO(hdr.bloom.scale.add) \
   DO(hdr.bloom.threshold.mult) \
   DO(hdr.bloom.threshold.add) \
   DO(hdr.eye_adapt_speed.mult) \
   DO(hdr.eye_adapt_speed.add) \
   DO(hdr.target_luminescence.min.mult) \
   DO(hdr.target_luminescence.min.add) \
   DO(hdr.target_luminescence.max.mult) \
   DO(hdr.target_luminescence.max.add) \
   DO(hdr.sky_scale.mult) \
   DO(hdr.sky_scale.add) \
   DO(hdr.sunlight_scale.mult) \
   DO(hdr.sunlight_scale.add) \
   DO(unknown[0].mult) \
   DO(unknown[0].add) \
   DO(unknown[1].mult) \
   DO(unknown[1].add) \
   DO(unknown[2].mult) \
   DO(unknown[2].add) \
   DO(unknown[3].mult) \
   DO(unknown[3].add) \
   DO(unknown[4].mult) \
   DO(unknown[4].add) \
   DO(unknown[5].mult) \
   DO(unknown[5].add) \
   DO(unknown[6].mult) \
   DO(unknown[6].add) \
   DO(unknown[7].mult) \
   DO(unknown[7].add) \
   DO(unknown[8].mult) \
   DO(unknown[8].add)

namespace ui::types::imagespace_modifier {
   void keyframe_collection::import_data(const dovah::loaded_forms::ImagespaceModifier& src) {
      this->duration = src.duration;
      this->keyframes.clear();

      using interpolated_color    = dovah::loaded_forms::ImagespaceModifier::interpolated_color;
      using interpolated_float    = dovah::loaded_forms::ImagespaceModifier::interpolated_float;
      using interpolated_mult_add = dovah::loaded_forms::ImagespaceModifier::interpolated_mult_add;

      auto _import_property = [this]<typename PropertyType>(const PropertyType& src_property, auto&& dst_getter) {
         if constexpr (std::is_same_v<PropertyType, interpolated_color>) {
            for (auto& frame : src_property) {
               auto& kf = this->get_or_create_keyframe(frame.time);
               dst_getter(kf) = QColor::fromRgbF(frame.value.r, frame.value.g, frame.value.b, frame.value.a);
            }
         } else {
            for (auto& frame : src_property) {
               auto& kf = this->get_or_create_keyframe(frame.time);
               dst_getter(kf) = frame.value;
            }
         }
      };
      #define X(prop, ...) _import_property(src.prop, [](keyframe& kf) constexpr -> auto& { return kf.prop; });
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

      auto _export_property = []<typename PropertyType>(float position, const auto& src_property, PropertyType & dst_property) {
         if constexpr (std::is_same_v<PropertyType, interpolated_color>) {
            if (src_property.has_value()) {
               auto& dst_frame   = dst_property.emplace_back();
               dst_frame.time    = position;
               dst_frame.value.r = src_property.value().redF();
               dst_frame.value.g = src_property.value().greenF();
               dst_frame.value.b = src_property.value().blueF();
               dst_frame.value.a = src_property.value().alphaF();
            }
         } else {
            if (src_property.has_value()) {
               auto& dst_frame = dst_property.emplace_back();
               dst_frame.time  = position;
               dst_frame.value = src_property.value();
            }
         }
      };
      for (const auto& src_keyframe : this->keyframes) {
         #define X(prop, ...) _export_property(src_keyframe.position, src_keyframe.prop, dst.prop);
         FOR_EACH_ANIMATED_PROPERTY(X);
         #undef X
      }
   }

   keyframe& keyframe_collection::get_or_create_keyframe(float position) {
      if (!this->keyframes.empty()) {
         auto it = std::upper_bound(
            this->keyframes.begin(),
            this->keyframes.end(),
            position,
            [](float desired, const keyframe& item) -> bool {
               return desired <= item.position;
            }
         );
         if (it != this->keyframes.end()) {
            if (it->position == position)
               return *it;
            assert(it->position > position);
            it = this->keyframes.insert(it, {});
            it->position = position;
            return *it;
         }
      }
      auto& kf = this->keyframes.emplace_back();
      kf.position = position;
      return kf;
   }
   keyframe& keyframe_collection::get_or_create_keyframe_at_timestamp(float timestamp) {
      float position = timestamp / this->duration;
      return this->get_or_create_keyframe(position);
   }

   void keyframe_collection::remove_keyframe_at_position(float position) {
      std::erase_if(
         this->keyframes,
         [position](const keyframe& kf) {
            return kf.position == position;
         }
      );
   }

   const keyframe* keyframe_collection::keyframe_at_position(float position) const noexcept {
      for (auto& kf : this->keyframes)
         if (kf.position == position)
            return &kf;
      return nullptr;
   }
   keyframe* keyframe_collection::keyframe_at_position(float position) noexcept {
      return const_cast<keyframe*>(std::as_const(*this).keyframe_at_position(position));
   }
   const keyframe* keyframe_collection::keyframe_at_timestamp(float timestamp) const noexcept {
      for (auto& kf : this->keyframes) {
         auto kf_time = kf.position * this->duration;
         if (kf_time == timestamp)
            return &kf;
      }
      return nullptr;
   }
   keyframe* keyframe_collection::keyframe_at_timestamp(float timestamp) noexcept {
      return const_cast<keyframe*>(std::as_const(*this).keyframe_at_timestamp(timestamp));
   }

   const keyframe* keyframe_collection::keyframe_before_position(float position) const noexcept {
      const keyframe* match = nullptr;
      for (auto& current : this->keyframes) {
         if (current.position < position)
            match = &current;
         else if (current.position >= position)
            break;
      }
      return match;
   }
   const keyframe* keyframe_collection::keyframe_after_position(float position) const noexcept {
      for (auto& current : this->keyframes)
         if (current.position > position)
            return &current;
      return nullptr;
   }

   computed_keyframe keyframe_collection::get_computed_keyframe(float at, bool is_timestamp) const noexcept {
      computed_keyframe interpolated;
      auto _interpolate_property = [this, at, is_timestamp, &interpolated](auto& dst_property, auto&& src_property_getter) {
         using  dst_property_type = std::decay_t<decltype(dst_property)>;
         struct found {
            float time = 0;
            dst_property_type value = {};
         };

         std::optional<found> found_prev;
         std::optional<found> found_next;
         for (const auto& src_keyframe : this->keyframes) {
            const auto& src_value_opt = src_property_getter(src_keyframe);
            if (!src_value_opt.has_value())
               continue;
            const auto  src_value = src_value_opt.value();
            auto src_time = src_keyframe.position;
            if (is_timestamp)
               src_time *= this->duration;
            if (src_time <= at) {
               if (!found_prev.has_value() || found_prev.value().time < src_time)
                  found_prev = found{ src_time, src_value };
            }
            if (src_time >= at) {
               if (!found_next.has_value() || found_next.value().time > src_time)
                  found_next = found{ src_time, src_value };
            }
         }
         if (found_prev.has_value() && found_next.has_value()) {
            const auto& a = found_prev.value();
            const auto& b = found_next.value();
            if (a.time == b.time) {
               dst_property = a.value;
               return;
            }
            float timespan = b.time - a.time;
            float factor = (at - found_prev.value().time) / timespan;
            float inv_factor = 1.0F - factor;
            if constexpr (std::is_same_v<dst_property_type, float>) {
               dst_property = (b.value * factor) + (a.value * inv_factor);
            } else if constexpr (std::is_same_v<dst_property_type, QColor>) {
               dst_property.setRedF((b.value.redF() * factor) + (a.value.redF() * inv_factor));
               dst_property.setGreenF((b.value.greenF() * factor) + (a.value.greenF() * inv_factor));
               dst_property.setBlueF((b.value.blueF() * factor) + (a.value.blueF() * inv_factor));
               dst_property.setAlphaF((b.value.alphaF() * factor) + (a.value.alphaF() * inv_factor));
            }
            return;
         }
         if (!found_prev.has_value() && !found_next.has_value())
            return;
         if (found_prev.has_value())
            dst_property = found_prev.value().value;
         else if (found_next.has_value())
            dst_property = found_next.value().value;
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
               if (kf.prop.has_value()) \
                  return false;
            FOR_EACH_ANIMATED_PROPERTY(X);
            #undef X
            return true;
         }
      );
   }
}