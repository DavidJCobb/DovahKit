#pragma once
#include <cmath>
#include <concepts>
#include <limits>
#include <type_traits>
#include <QDoubleSpinBox>
#include <QSpinBox>

namespace ui {
   namespace _impl {
      template<typename T>
      concept arithmetic = std::is_arithmetic_v<T>;

      template<typename Widget>
      concept spinbox = requires (const Widget& cw, Widget& w) {
         requires std::is_base_of_v<QAbstractSpinBox, Widget>;
         { cw.value() } -> arithmetic<>;
         { w.setMinimum(cw.value()) } -> std::same_as<void>;
         { w.setMaximum(cw.value()) } -> std::same_as<void>;
         { w.setRange(cw.value(), cw.value()) } -> std::same_as<void>;
      };
      
      template<typename Widget>
      concept floating_point_spinbox = requires (const Widget& cw, Widget& w, float f, int i) {
         requires spinbox<Widget>;
         { cw.value() } -> std::floating_point<>;
         { cw.decimals() } -> std::convertible_to<int>;
         { w.setDecimals(cw.decimals()) } -> std::same_as<void>;
      };

      template<typename Widget>
      concept spinbox_has_step = requires(const Widget & cw, Widget & w) {
         requires spinbox<Widget>;
         { cw.singleStep() };
         { w.setSingleStep(cw.singleStep()) } -> std::same_as<void>;
      };

      template<typename Widget, typename Value>
      concept spinbox_can_represent =
         spinbox<Widget> &&
         arithmetic<Value> &&
         (floating_point_spinbox<Widget>    || !std::is_floating_point_v<Widget>) && // Need a floating-point spinbox for float values.
         (!std::is_same_v<Widget, QSpinBox> || std::numeric_limits<Value>::max() <= std::numeric_limits<int>::max()) // QSpinBox is capped to the range of the int type.
      ;

      template<spinbox Widget>
      using spinbox_value_type = decltype(Widget().value());

      template<floating_point_spinbox Spinbox>
      extern void force_spinbox_value_to_int(Spinbox* widget) {
         spinbox_value_type<Spinbox> integer_part = {};
         if (std::modf(widget->value(), &integer_part) != 0) {
            widget->setValue(integer_part);
         }
         if constexpr (spinbox_has_step<Spinbox>) {
            if (std::modf(widget->singleStep(), &integer_part) != 0) {
               widget->setSingleStep(integer_part);
            }
         }
      }
   }

   //
   // Set a spinbox widget's minimum and maximum values based on those of a 
   // given arithmetic type.
   //
   template<typename Value, _impl::spinbox Spinbox>
   extern void set_range(Spinbox* widget) requires _impl::spinbox_can_represent<Spinbox, Value> {
      if constexpr (_impl::floating_point_spinbox<Spinbox>) {
         if (!std::is_floating_point_v<Value>) {
            widget->setDecimals(0);
            _impl::force_spinbox_value_to_int(widget);
         }
      }
      widget->setRange(
         std::numeric_limits<Value>::lowest(),
         std::numeric_limits<Value>::max()
      );
   }

   //
   // Set a spinbox widget's minimum value to zero, and its maximum value to 
   // that of a given arithmetic type. This function is mainly useful for 
   // floats, which have no unsigned types.
   //
   template<typename Value, _impl::spinbox Spinbox>
   extern void set_unsigned_range(Spinbox* widget) requires _impl::spinbox_can_represent<Spinbox, Value> {
      if constexpr (_impl::floating_point_spinbox<Spinbox>) {
         if (!std::is_floating_point_v<Value>) {
            widget->setDecimals(0);
            _impl::force_spinbox_value_to_int(widget);
         }
      }
      widget->setRange(
         0,
         std::numeric_limits<Value>::max()
      );
   }
}