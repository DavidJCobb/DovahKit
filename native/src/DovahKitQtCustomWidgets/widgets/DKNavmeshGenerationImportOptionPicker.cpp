#include "./DKNavmeshGenerationImportOptionPicker.h"
#include <array>
#include <QBoxLayout>

DKNavmeshGenerationImportOptionPicker::DKNavmeshGenerationImportOptionPicker(QWidget* parent) : QWidget(parent) {
   auto* widget = this->_subwidgets.combobox = new QComboBox(this);
   widget->addItem(tr("Collision"),    (uint)Value::CollisionGeometry);
   widget->addItem(tr("Bounding box"), (uint)Value::BoundingBox);
   widget->addItem(tr("Filter"),       (uint)Value::Filter);
   widget->addItem(tr("Ground"),       (uint)Value::Ground);

   auto* layout = new QHBoxLayout(this);
   this->setLayout(layout);
   layout->setContentsMargins(0, 0, 0, 0);
   layout->addWidget(widget);

   QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
      emit this->valueChanged(this->getValue());
   });

   this->setFocusPolicy(Qt::FocusPolicy::TabFocus);
   this->setFocusProxy(widget);
}

DKNavmeshGenerationImportOptionPicker::Value DKNavmeshGenerationImportOptionPicker::getValue() const {
   return (Value) this->_subwidgets.combobox->currentData().toInt();
}
void DKNavmeshGenerationImportOptionPicker::setValue(Value v) {
   if (this->getValue() == v)
      return;

   auto i = this->_subwidgets.combobox->findData((int)v);
   if (i >= 0)
      this->_subwidgets.combobox->setCurrentIndex(i);
}

void DKNavmeshGenerationImportOptionPicker::setValueByMask(uint32_t mask) {
   constexpr const auto all_flags = std::array{
      Value::CollisionGeometry,
      Value::BoundingBox,
      Value::Filter,
      Value::Ground,
   };
   for (auto flag : all_flags) {
      if (mask & (uint32_t)flag) {
         this->setValue(flag);
         return;
      }
   }

   this->setValue(Value::CollisionGeometry);
}
void DKNavmeshGenerationImportOptionPicker::writeValueToMask(uint32_t& dst) const {
   dst &= ~(
      (uint32_t)Value::CollisionGeometry |
      (uint32_t)Value::BoundingBox |
      (uint32_t)Value::Filter |
      (uint32_t)Value::Ground
   );
   dst |= (uint32_t) this->getValue();
}