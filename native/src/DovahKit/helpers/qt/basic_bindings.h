#pragma once
#include "../bitwise.h"
#include <string>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>

namespace cobb::qt {
   extern void bind(QCheckBox*, bool&);
   extern void unbind(QCheckBox*); // just severs all QCheckBox::stateChanged where the sender and receiver are the same
   //
   template<typename T, typename M> extern void bind(QCheckBox* widget, T& target, M mask) {
      widget->setChecked(target & mask ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
      QObject::connect(widget, &QCheckBox::stateChanged, widget, [&target, mask](int state) {
         cobb::edit_bit(target, mask, state == Qt::CheckState::Checked);
      });
   }

   extern void bind(QLineEdit*, std::string&);
   extern void unbind(QLineEdit*);

   template<typename T> extern void bind(QComboBox* widget, T& target) {
      if constexpr (std::is_enum_v<T>) {
         static_assert(sizeof(T) <= sizeof(int), "This template won't work for enums larger than an int.");
         widget->setCurrentIndex(widget->findData((int)target));
         QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), widget, [widget, &target](int index) {
            target = (T)widget->currentData().toInt();
         });
      } else {
         widget->setCurrentIndex(widget->findData(target));
         QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), widget, [widget, &target](int index) {
            target = widget->currentData().toInt();
         });
      }
   }
   extern inline void unbind(QComboBox* widget) {
      QObject::disconnect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), widget, nullptr);
   }

   template<typename T> extern void bind(QSpinBox* widget, T& target) {
      widget->setValue(target);
      QObject::connect(widget, QOverload<int>::of(&QSpinBox::valueChanged), widget, [widget, &target](int i) {
         target = i;
      });
   }
   extern inline void unbind(QSpinBox* widget) {
      QObject::disconnect(widget, QOverload<int>::of(&QSpinBox::valueChanged), widget, nullptr);
   }

   template<typename T> extern void bind(QDoubleSpinBox* widget, T& target) {
      widget->setValue(target);
      QObject::connect(widget, QOverload<double>::of(&QDoubleSpinBox::valueChanged), widget, [widget, &target](double f) {
         target = f;
      });
   }
   extern inline void unbind(QDoubleSpinBox* widget) {
      QObject::disconnect(widget, QOverload<double>::of(&QDoubleSpinBox::valueChanged), widget, nullptr);
   }
}