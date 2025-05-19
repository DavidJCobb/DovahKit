#include "./DKYesNoUnsetWidget.h"
#include <array>
#include <QEvent>
#include <QHBoxLayout>
#include <QStyle>
#include <QStyleOption>

DKYesNoUnsetWidget::DKYesNoUnsetWidget(QWidget* parent) : QWidget(parent) {
   this->_subwidgets.checkbox   = new QCheckBox(this);
   this->_subwidgets.pushbutton = new QPushButton(this);
   {
      auto* layout = new QHBoxLayout(this);
      layout->setContentsMargins(0, 0, 0, 0);
      layout->addWidget(this->_subwidgets.pushbutton);
      layout->addWidget(this->_subwidgets.checkbox);
   }
   this->_subwidgets.pushbutton->setCheckable(true);
   this->_subwidgets.checkbox->setEnabled(false);

   QObject::connect(this->_subwidgets.pushbutton, &QPushButton::toggled, this, [this](bool checked) {
      this->_subwidgets.checkbox->setEnabled(checked);
      emit this->stateChanged(this->checkState());
   });
   QObject::connect(this->_subwidgets.checkbox, &QCheckBox::toggled, this, [this](bool checked) {
      emit this->stateChanged(this->checkState());
   });
}

Qt::CheckState DKYesNoUnsetWidget::checkState() const {
   if (!this->_subwidgets.pushbutton->isChecked())
      return Qt::CheckState::PartiallyChecked;
   return this->_subwidgets.checkbox->checkState();
}
void DKYesNoUnsetWidget::setCheckState(Qt::CheckState s) {
   if (s == this->checkState()) {
      return;
   }
   {
      const auto blockers = std::array{
         QSignalBlocker(this->_subwidgets.pushbutton),
         QSignalBlocker(this->_subwidgets.checkbox),
      };
      switch (s) {
         case Qt::CheckState::PartiallyChecked:
            this->_subwidgets.pushbutton->setChecked(false);
            this->_subwidgets.checkbox->setEnabled(false);
            break;
         case Qt::CheckState::Checked:
            this->_subwidgets.pushbutton->setChecked(true);
            this->_subwidgets.checkbox->setEnabled(true);
            this->_subwidgets.checkbox->setChecked(true);
            break;
         case Qt::CheckState::Unchecked:
            this->_subwidgets.pushbutton->setChecked(true);
            this->_subwidgets.checkbox->setEnabled(true);
            this->_subwidgets.checkbox->setChecked(false);
            break;
      }
   }
   emit this->stateChanged(this->checkState());
}

QString DKYesNoUnsetWidget::text() const {
   return this->_subwidgets.checkbox->text();
}
void DKYesNoUnsetWidget::setText(QString t) {
   this->_subwidgets.checkbox->setText(t);
}

#pragma region Overrides
   /*virtual*/ bool DKYesNoUnsetWidget::event(QEvent* event) /*override*/ {
      if (event->type() == QEvent::Polish) {
         QStyleOption option;
         option.initFrom(this->_subwidgets.checkbox);

         auto* style = this->style();
         auto  rect  = style->subElementRect(QStyle::SubElement::SE_CheckBoxIndicator, &option, this->_subwidgets.checkbox);

         this->_subwidgets.pushbutton->setFixedSize(rect.size());
      }
      return QWidget::event(event);
   }
#pragma endregion