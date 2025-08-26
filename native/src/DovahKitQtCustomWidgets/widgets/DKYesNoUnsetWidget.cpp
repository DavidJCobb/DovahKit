#include "./DKYesNoUnsetWidget.h"
#include <array>
#include <QEvent>
#include <QHBoxLayout>
#include <QStyle>
#include <QStyleOption>
#include <QStylePainter>

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

   this->setFocusPolicy(Qt::FocusPolicy::TabFocus);
   this->setFocusProxy(this->_subwidgets.pushbutton);
   QWidget::setTabOrder(this->_subwidgets.pushbutton, this->_subwidgets.checkbox);

   this->setSizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Fixed);

   QObject::connect(this->_subwidgets.pushbutton, &QPushButton::toggled, this, [this](bool checked) {
      this->_subwidgets.checkbox->setEnabled(checked);
      emit this->stateChanged(this->checkState());
   });
   QObject::connect(this->_subwidgets.checkbox, &QCheckBox::toggled, this, [this](bool checked) {
      emit this->stateChanged(this->checkState());
   });

   this->_subwidgets.checkbox->installEventFilter(this);
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
   #if defined(QT_PLUGIN)
      /*virtual*/ QRect DKYesNoUnsetWidget::inPlaceTextEditingBounds() const /*override*/ {
         QStyleOptionButton option;
         this->_initCheckboxStyleOption(option);
         auto* style = this->style();
         auto  rect  = style->subElementRect(QStyle::SubElement::SE_CheckBoxContents, &option, this->_subwidgets.checkbox);

         QRect geom;
         geom.setTopLeft(this->_subwidgets.checkbox->geometry().topLeft() + rect.topLeft());
         geom.setHeight(rect.height());
         geom.setWidth(this->contentsRect().width());
         return geom;
      }
   #endif

   /*virtual*/ QSize DKYesNoUnsetWidget::sizeHint() const /*override*/ {
      auto hint_a = this->_subwidgets.pushbutton->size();
      auto hint_b = this->_subwidgets.checkbox->sizeHint();
      auto margin = this->layout()->contentsMargins();
      auto gap    = this->layout()->spacing();

      QSize size;
      size.setWidth(hint_a.width() + gap + hint_b.width() + margin.left() + margin.right());
      size.setHeight(std::max(hint_a.height(), hint_b.height()) + margin.top() + margin.bottom());
      return size;
   }
   /*virtual*/ QSize DKYesNoUnsetWidget::minimumSizeHint() const /*override*/ {
      return this->sizeHint();
   }

   /*virtual*/ bool DKYesNoUnsetWidget::event(QEvent* event) /*override*/ {
      if (event->type() == QEvent::Polish) {
         QStyleOptionButton option;
         this->_initCheckboxStyleOption(option);

         auto* style = this->style();
         auto  rect  = style->subElementRect(QStyle::SubElement::SE_CheckBoxIndicator, &option, this->_subwidgets.checkbox);

         this->_subwidgets.pushbutton->setFixedSize(rect.size());
      }
      return QWidget::event(event);
   }
   /*virtual*/ bool DKYesNoUnsetWidget::eventFilter(QObject* watched, QEvent* event) /*override*/ {
      //
      // When we're in the "unset" state, we want the checkbox sub-widget to be disabled. 
      // However, we want its *label text* to be drawn enabled, unless we ourselves are 
      // disabled.
      // 
      // In essence, we're using the checkbox text as *our* text, and we want it drawn 
      // accordingly.
      //
      if (watched == this->_subwidgets.checkbox && event->type() == QEvent::Type::Paint) {
         auto* casted = (QPaintEvent*)event;
         QStylePainter      painter(this->_subwidgets.checkbox);
         QStyleOptionButton option;
         this->_initCheckboxStyleOption(option);
         {
            const auto* widget = this->_subwidgets.checkbox;
            const auto* style  = widget->style();
            const auto* proxy  = style->proxy();
            {
               QStyleOptionButton part_option = option;

               // The checkbox itself.
               part_option.rect = style->subElementRect(QStyle::SE_CheckBoxIndicator, &option, widget);
               proxy->drawPrimitive(QStyle::PE_IndicatorCheckBox, &part_option, &painter, widget);

               // The label.
               part_option.rect = style->subElementRect(QStyle::SE_CheckBoxContents, &option, widget);
               if (this->isEnabled()) {
                  part_option.state |= QStyle::StateFlag::State_Enabled;
                  part_option.palette.setCurrentColorGroup(this->palette().currentColorGroup());
               }
               proxy->drawControl(QStyle::CE_CheckBoxLabel, &part_option, &painter, widget);
            }

            // The focus ring.
            if (option.state & QStyle::State_HasFocus) {
               QStyleOptionFocusRect focus_rect_option;
               ((QStyleOption&)focus_rect_option) = (QStyleOption&)option;
               focus_rect_option.rect = style->subElementRect(QStyle::SE_CheckBoxFocusRect, &option, widget);
               proxy->drawPrimitive(QStyle::PE_FrameFocusRect, &focus_rect_option, &painter, widget);
            }
         }
         //painter.drawControl(QStyle::CE_CheckBox, option);
         return true;
      }
      return QObject::eventFilter(watched, event);
   }
#pragma endregion

void DKYesNoUnsetWidget::_initCheckboxStyleOption(QStyleOptionButton& dst) const {
   const auto* widget = this->_subwidgets.checkbox;
   dst.initFrom(widget);
   dst.text     = this->text();
   dst.icon     = widget->icon();
   dst.iconSize = widget->iconSize();
   switch (this->checkState()) {
      case Qt::CheckState::Checked:
         dst.state |= QStyle::State_On;
         break;
      case Qt::CheckState::PartiallyChecked:
         dst.state |= QStyle::State_NoChange;
         break;
      case Qt::CheckState::Unchecked:
         dst.state |= QStyle::State_Off;
         break;
   }
}