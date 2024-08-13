#pragma once
#include <bit>
#include <string>
#include <type_traits>
#include <vector>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QSlider>
#include <QSpinBox>
#include <QLineEdit>

namespace dovah{
   namespace loaded_forms {
      class Form;

      union color_t;
   }
   class form_reference_t;
   class form_stub;
}
class DKColorPickerButton;
class DKCompactObjectReferencePicker;
class DKFloatSlider;
class DKFormPicker;
class DKGameFilePicker;
class DKNavmeshGenerationImportOptionPicker;
class DKObjectReferencePicker;

namespace ui {
   extern void bind(QCheckBox*, bool&);
   extern void bind(QGroupBox*, bool&);

   template<typename Target, typename Mask>
   void bind(QCheckBox* widget, Target& target, Mask mask) {
      widget->setChecked((target & mask) != 0);
      QObject::connect(widget, &QCheckBox::stateChanged, widget, [&target, mask](int state) {
         if (state == Qt::CheckState::Checked)
            target |= mask;
         else
            target &= ~mask;
      });
   }
   //
   template<typename Target, typename Mask>
   void bind(QGroupBox* widget, Target& target, Mask mask) {
      widget->setChecked((target & mask) != 0);
      QObject::connect(widget, &QGroupBox::toggled, widget, [&target, mask](bool checked) {
         if (checked)
            target |= mask;
         else
            target &= ~mask;
      });
   }

   //
   // Bind the opposite of a checkbox's state to a given flag.
   //
   template<typename Target, typename Mask>
   void bind_inverse(QCheckBox* widget, Target& target, Mask mask) {
      widget->setChecked((target & mask) == 0);
      QObject::connect(widget, &QCheckBox::stateChanged, widget, [&target, mask](int state) {
         if (state == Qt::CheckState::Checked)
            target &= ~mask;
         else
            target |= mask;
      });
   }
   //
   template<typename Target, typename Mask>
   void bind_inverse(QGroupBox* widget, Target& target, Mask mask) {
      widget->setChecked((target & mask) == 0);
      QObject::connect(widget, &QGroupBox::toggled, widget, [&target, mask](bool checked) {
         if (checked)
            target &= ~mask;
         else
            target |= mask;
      });
   }

   //
   // Bind a list of radio buttons to some, but not all, of the bits in a flags mask. 
   // A call to this function will generally look like this:
   // 
   //    ui::bind(
   //       target_flags_mask,
   //       std::array{
   //          std::pair{ this->ui.radioButtonA, 0x0010 },
   //          std::pair{ this->ui.radioButtonB, 0x0100 },
   //          std::pair{ this->ui.radioButtonC, 0x0110 },
   //          std::pair{ nullptr,               0x1000 }, // perhaps this value isn't exposed in the UI yet
   //       }
   //    );
   //
   template<typename Target, typename Mask, size_t Count>
   void bind_flags(Target& target, const std::array<std::pair<QRadioButton*, Mask>, Count>& pair) {
      Mask all_bits = 0;
      for (const auto& item : pair)
         all_bits |= item.second;

      QRadioButton* target_widget   = nullptr;
      size_t        target_bitcount = 0;

      for (const auto& item : pair) {
         auto* widget = item.first;
         auto  value  = item.second;
         if (!widget)
            continue;

         QObject::connect(widget, &QRadioButton::toggled, widget, [all_bits, value, &target](bool checked) {
            if (!checked)
               return;
            target &= ~all_bits;
            target |= value;
         });

         //
         // We want to set the currently checked radio button to the widget whose value 
         // best matches the set bits in the target value. A value matches if all of its
         // bits are set; the best match is the match that has the most set bits.
         //
         size_t bits_set = target & value;
         if (bits_set == value) {
            auto pc = std::popcount(value);
            if (pc > target_bitcount) {
               target_bitcount = pc;
               target_widget   = widget;
            }
         }
      }
      if (target_widget) {
         target_widget->setChecked(true);
      }
   }
   
   //
   // Bind an enum to a QComboBox, such that you can just store the values as int-type 
   // user-data without having to set up the enum for Qt's meta-type system.
   //
   template<typename Target> requires std::is_enum_v<Target>
   void bind(QComboBox* widget, Target& target) {
      static_assert(sizeof(Target) <= sizeof(int), "This template won't work for enums larger than an int.");
      widget->setCurrentIndex(widget->findData((int)target));
      QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), widget, [widget, &target](int index) {
         target = (Target)widget->currentData().toInt();
      });
   }

   //
   // Bind an enum to a QComboBox, such that you can just store the values as int-type 
   // user-data without having to set up the enum for Qt's meta-type system. Alternate 
   // template for when the value is stored as the enum's underlying type.
   //
   template<typename Target, typename TargetIntegral> requires (std::is_enum_v<Target> && std::is_same_v<std::underlying_type_t<Target>, TargetIntegral>)
   void bind(QComboBox* widget, TargetIntegral& target) {
      static_assert(sizeof(Target) <= sizeof(int), "This template won't work for enums larger than an int.");
      widget->setCurrentIndex(widget->findData((int)target));
      QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), widget, [widget, &target](int index) {
         target = (TargetIntegral)widget->currentData().toInt();
      });
   }

   template<typename Target> requires std::is_floating_point_v<Target>
   void bind(QDoubleSpinBox* widget, Target& target) {
      widget->setValue(target);
      QObject::connect(widget, QOverload<double>::of(&QDoubleSpinBox::valueChanged), widget, [widget, &target](double f) {
         target = f;
      });
   }

   template<typename Target> requires (std::is_arithmetic_v<Target> && !std::is_floating_point_v<Target>)
   void bind(QSlider* widget, Target& target) {
      widget->setValue(target);
      QObject::connect(widget, QOverload<int>::of(&QSlider::valueChanged), widget, [widget, &target](int v) {
         target = (Target)v;
      });
   }
   //
   template<typename Target, typename Factor> requires (std::is_floating_point_v<Target> && std::is_arithmetic_v<Factor>)
   void bind(QSlider* widget, Target& target, Factor factor) {
      widget->setValue(round(target * factor));
      QObject::connect(widget, QOverload<int>::of(&QSlider::valueChanged), widget, [widget, &target, factor](int v) {
         target = (Target)v / factor;
      });
   }

   // Assumes UTF-8.
   extern void bind(QLineEdit*, std::string&);
   
   template<typename Target> requires std::is_integral_v<Target>
   void bind(QSpinBox* widget, Target& target) {
      widget->setValue(target);
      QObject::connect(widget, QOverload<int>::of(&QSpinBox::valueChanged), widget, [widget, &target](int i) {
         target = i;
      });
   }

   extern void bind(DKColorPickerButton*, dovah::loaded_forms::color_t&);

   extern void bind(DKFormPicker*, dovah::form_stub*&);

   extern void bind(DKFloatSlider*, float&);

   // Intended for form working copies only. Making real-time changes to forms is bad UI/UX and 
   // you should use an OK/Cancel button instead. Once we do the backend rewrite to form working 
   // copies, you'll be able to just use the form_stub-pointer overload.
   //
   // This function will assert that the form you pass in is a working copy!
   extern void bind(DKFormPicker*, dovah::form_reference_t& dst, dovah::loaded_forms::Form& dst_owner);
   extern void bind(DKCompactObjectReferencePicker*, dovah::form_reference_t& dst, dovah::loaded_forms::Form& dst_owner);
   extern void bind(DKObjectReferencePicker*, dovah::form_reference_t& dst, dovah::loaded_forms::Form& dst_owner);
   
   extern void bind(DKGameFilePicker*, std::string&);

   extern void bind(DKNavmeshGenerationImportOptionPicker*, uint32_t& record_flags);
}
