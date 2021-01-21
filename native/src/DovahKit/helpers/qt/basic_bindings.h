#pragma once
#include "../bitwise.h"
#include <string>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>

namespace cobb::qt {
   void bind(QCheckBox*, bool&);
   void unbind(QCheckBox*); // just severs all QCheckBox::stateChanged where the sender and receiver are the same

   template<typename T, typename M> void bind(QCheckBox* widget, T& target, M mask) {
      widget->setChecked(target & mask ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
      QObject::connect(widget, &QCheckBox::stateChanged, widget, [&target](int state) {
         cobb::edit_bit(target, mask, state == Qt::CheckState::Checked);
      });
   }

   void bind(QLineEdit*, std::string&);
   void unbind(QLineEdit*);

   template<typename T> void bind(QComboBox* widget, T& target) {
      widget->setCurrentIndex(widget->findData(target));
      QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), widget, [widget, &target](int index) {
         target = widget->currentData().toInt();
      });
   }
   void unbind(QComboBox* widget) {
      QObject::disconnect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), widget, nullptr);
   }

   template<typename T> void bind(QSpinBox* widget, T& target) {
      widget->setCurrentValue(target);
      QObject::connect(widget, QOverload<int>::of(&QSpinBox::valueChanged), widget, [widget, &target](int i) {
         target = i;
      });
   }
   void unbind(QSpinBox* widget) {
      QObject::disconnect(widget, QOverload<int>::of(&QSpinBox::valueChanged), widget, nullptr);
   }
}