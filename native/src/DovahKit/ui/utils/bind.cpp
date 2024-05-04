#include "./bind.h"
#include <cassert>
#include <QCheckBox>
#include <QLineEdit>
#include "dovah/forms/structs/color_dword.h"
#include "dovah/forms/Form.h" // for working copies
#include "dovah/core.h" // form_reference_t
#include "widgets/DKColorPickerButton.h"
#include "widgets/DKFormPicker.h"
#include "widgets/DKFormListPane.h"

namespace ui {
   extern void bind(QCheckBox* widget, bool& target) {
      widget->setChecked(target);
      QObject::connect(widget, &QCheckBox::stateChanged, widget, [&target](int state) {
         target = state == Qt::CheckState::Checked;
      });
   }

   extern void bind(QLineEdit* widget, std::string& target) {
      widget->setText(QString::fromUtf8(QByteArray::fromStdString(target)));
      QObject::connect(widget, &QLineEdit::textChanged, widget, [&target](const QString& value) {
         target = value.toUtf8().toStdString();
      });
   }

   extern void bind(DKColorPickerButton* widget, dovah::loaded_forms::color_t& target) {
      widget->setColor(QColor::fromRgb(
         target.r,
         target.g,
         target.b
      ));
      QObject::connect(widget, &DKColorPickerButton::colorChanged, widget, [widget, &target](QColor color) {
         target.r = color.red();
         target.g = color.green();
         target.b = color.blue();
         if (widget->hasAlpha()) {
            target.unused = color.alpha();
         }
      });
   }

   extern void bind(DKFormPicker* widget, dovah::form_stub*& target) {
      widget->setFormStub(target);
      QObject::connect(widget, &DKFormPicker::formChanged, widget, [&target](dovah::form_stub* value) {
         target = value;
      });
   }

   extern void bind(DKFormPicker* widget, dovah::form_reference_t& dst, dovah::loaded_forms::Form& dst_owner) {
      assert(dst_owner.is_working_copy && "This function was created to make things easier for the (messy) form-working-copy system. Don't use it for real forms.");
      widget->setFormStub(dst.get_form_stub());
      QObject::connect(widget, &DKFormPicker::formChanged, widget, [&dst, &dst_owner](dovah::form_stub* value) {
         dst.set(dst_owner, value);
      });
   }
}