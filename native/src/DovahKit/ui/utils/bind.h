#pragma once
#include <string>
#include <type_traits>
#include <vector>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
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
class DKFormPicker;
class DKNavmeshGenerationImportOptionPicker;

namespace ui {
   extern void bind(QCheckBox*, bool&);

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

   // Intended for form working copies only. Making real-time changes to forms is bad UI/UX and 
   // you should use an OK/Cancel button instead. Once we do the backend rewrite to form working 
   // copies, you'll be able to just use the form_stub-pointer overload.
   //
   // This function will assert that the form you pass in is a working copy!
   extern void bind(DKFormPicker*, dovah::form_reference_t& dst, dovah::loaded_forms::Form& dst_owner);

   extern void bind(DKNavmeshGenerationImportOptionPicker*, uint32_t& record_flags);
}
