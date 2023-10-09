#include "./WorldeditToolRangeInputScalesWidget.h"

WorldeditToolRangeInputScalesWidget::WorldeditToolRangeInputScalesWidget(QWidget* parent) : QWidget(parent) {
   this->ui.setupUi(this);

   {
      auto axes = std::array{ this->ui.xAxis, this->ui.yAxis };
      for (auto* widget : axes) {
         widget->addItem(tr("X"), (int)data_type::axis3D::x);
         widget->addItem(tr("Y"), (int)data_type::axis3D::y);
         widget->addItem(tr("Z"), (int)data_type::axis3D::z);
      }
   }
   {
      auto signs = std::array{ this->ui.xSign, this->ui.ySign };
      for (auto* widget : signs) {
         widget->addItem(tr("Positive"), (int)data_type::sign::positive);
         widget->addItem(tr("Negative"), (int)data_type::sign::negative);
      }
   }

   {
      auto widgets = std::array{
         this->ui.xAxis,
         this->ui.xSign,
         this->ui.yAxis,
         this->ui.ySign,
      };
      for (auto* widget : widgets) {
         QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int currentIndex) {
            if (this->sync_target)
               *this->sync_target = this->data();
         });
      }
   }
}

std::optional<WorldeditToolRangeInputScalesWidget::data_type> WorldeditToolRangeInputScalesWidget::data() const {
   if (this->isOptional() && !this->ui.groupbox->isChecked())
      return {};
   return data_type{
      .x = {
         .axis = (data_type::axis3D) this->ui.xAxis->currentData().toInt(),
         .sign = (data_type::sign)   this->ui.xSign->currentData().toInt()
      },
      .y = {
         .axis = (data_type::axis3D) this->ui.yAxis->currentData().toInt(),
         .sign = (data_type::sign)   this->ui.ySign->currentData().toInt()
      },
   };
}
void WorldeditToolRangeInputScalesWidget::setData(const std::optional<data_type>& v) {
   if (this->isOptional()) {
      if (!v.has_value()) {
         this->ui.groupbox->setChecked(false);
         return;
      }
      this->ui.groupbox->setChecked(true);
   } else {
      if (!v.has_value())
         return;
   }
   this->setData(v.value());
}
void WorldeditToolRangeInputScalesWidget::setData(const data_type& v) {
   this->ui.xAxis->setCurrentIndex(this->ui.xAxis->findData((int)v.x.axis));
   this->ui.xSign->setCurrentIndex(this->ui.xSign->findData((int)v.x.sign));
   this->ui.yAxis->setCurrentIndex(this->ui.yAxis->findData((int)v.y.axis));
   this->ui.ySign->setCurrentIndex(this->ui.ySign->findData((int)v.y.sign));
}

bool WorldeditToolRangeInputScalesWidget::isOptional() const {
   return this->ui.groupbox->isCheckable();
}
void WorldeditToolRangeInputScalesWidget::setIsOptional(bool v) {
   this->ui.groupbox->setCheckable(v);
   if (!v)
      this->ui.groupbox->setChecked(true);
}

QString WorldeditToolRangeInputScalesWidget::label() const {
   return this->ui.groupbox->title();
}
void WorldeditToolRangeInputScalesWidget::setLabel(const QString v) {
   this->ui.groupbox->setTitle(v);
}

const std::optional<QString>& WorldeditToolRangeInputScalesWidget::_override_for_axis(data_type::axis3D axis) const {
   switch (axis) {
      using enum data_type::axis3D;
      case x: return this->overrides.x;
      case y: return this->overrides.y;
      case z: return this->overrides.z;
   }
   std::unreachable();
}
std::optional<QString>& WorldeditToolRangeInputScalesWidget::_override_for_axis(data_type::axis3D axis) {
   return const_cast<std::optional<QString>&>(std::as_const(*this)._override_for_axis(axis));
}

std::optional<QString> WorldeditToolRangeInputScalesWidget::axisNameOverride(data_type::axis3D axis) const {
   return this->_override_for_axis(axis);
}
void WorldeditToolRangeInputScalesWidget::clearAxisNameOverride(data_type::axis3D axis) {
   auto& target = this->_override_for_axis(axis);
   if (!target.has_value())
      return;
   target = {};

   {
      const char* text = "?";
      switch (axis) {
         using enum data_type::axis3D;
         case x: text = "X"; break;
         case y: text = "Y"; break;
         case z: text = "Z"; break;
      }

      auto widgets = std::array{ this->ui.xAxis, this->ui.yAxis };
      for (auto* widget : widgets) {
         auto i = widget->findData((int)axis);
         if (i < 0)
            continue;
         widget->setItemText(i, text);
      }
   }
}
void WorldeditToolRangeInputScalesWidget::setAxisNameOverride(data_type::axis3D axis, QString name) {
   auto& target = this->_override_for_axis(axis);
   if (target == name)
      return;
   target = name;

   {
      auto widgets = std::array{ this->ui.xAxis, this->ui.yAxis };
      for (auto* widget : widgets) {
         auto i = widget->findData((int)axis);
         if (i < 0)
            continue;
         widget->setItemText(i, name);
      }
   }
}

void WorldeditToolRangeInputScalesWidget::reloadFromSyncTarget() {
   if (!this->sync_target)
      return;

   const auto blocker_a = QSignalBlocker(this->ui.xAxis);
   const auto blocker_b = QSignalBlocker(this->ui.xSign);
   const auto blocker_c = QSignalBlocker(this->ui.yAxis);
   const auto blocker_d = QSignalBlocker(this->ui.ySign);

   this->setData(*this->sync_target);
}
void WorldeditToolRangeInputScalesWidget::setSyncTarget(std::optional<data_type>* t) {
   if (this->sync_target == t)
      return;
   this->sync_target = t;
   if (t) {
      *t = this->data();
   }
}

void WorldeditToolRangeInputScalesWidget::adjustForRangeInput(dovahkit::subsystems::worldinput::range_input_control ctrl, dovahkit::subsystems::worldinput::range_input_axes axes) {
   using namespace dovahkit::subsystems::worldinput;

   bool any_control = ctrl != range_input_control::none;
   this->ui.groupbox->setEnabled(any_control);

   bool control_has_two_axes = !any_control || range_input_control_has_multiple_axes(ctrl);
   if (!control_has_two_axes) {
      axes = range_input_axes::x;
   }

   bool x_visible = true;
   bool y_visible = true;
   if (axes != range_input_axes::all) {
      x_visible = (axes == range_input_axes::x);
      y_visible = !x_visible;
   }

   this->ui.xLabel->setVisible(x_visible);
   this->ui.xAxis->setVisible(x_visible);
   this->ui.xSign->setVisible(x_visible);

   this->ui.yLabel->setVisible(y_visible);
   this->ui.yAxis->setVisible(y_visible);
   this->ui.ySign->setVisible(y_visible);

   if (!control_has_two_axes) {
      this->ui.xLabel->setText(tr("Input Axis:"));
   } else {
      this->ui.xLabel->setText(tr("Input X-Axis:"));
   }
}