#include "./qpalette.h"
#include <unordered_map>
#include <QApplication>
#include <QComboBox>
#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QPalette>
#include "widgets/DKColorPickerButton.h"

namespace DovahKitDebug::features::qt {
   /*static*/ void qpalette::execute(QWidget* from) {
      auto* dialog = new QDialog(from);
      auto* layout = new QGridLayout(dialog);
      QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);

      int row = 0;
      std::unordered_map<QPalette::ColorRole, DKColorPickerButton*> buttons;

      QComboBox* group_picker = nullptr;
      {
         auto* label  = new QLabel("Color group:", dialog);
         auto* widget = new QComboBox(dialog);
         layout->addWidget(label,  row, 0, {});
         layout->addWidget(widget, row, 1);
         ++row;

         group_picker = widget;

         widget->addItem("Normal",   QPalette::ColorGroup::Normal);
         widget->addItem("Active",   QPalette::ColorGroup::Active);
         widget->addItem("Inactive", QPalette::ColorGroup::Inactive);
         widget->addItem("Disabled", QPalette::ColorGroup::Disabled);
      }

      {
         auto _add_button = [dialog, layout, &row, &buttons](QPalette::ColorRole role, std::string_view text) {
            auto* label = new QLabel(QString(text.data()), dialog);
            auto* color = new DKColorPickerButton(dialog);
            buttons[role] = color;
            color->setHasAlpha(true);
            color->setProperty("role", role);
            layout->addWidget(label, row, 0, {});
            layout->addWidget(color, row, 1);
            ++row;
         };
         auto _add_header = [dialog, layout, &row](QString text) {
            auto* label = new QLabel(text, dialog);
            auto  font = label->font();
            font.setBold(true);
            label->setFont(font);
            layout->addWidget(label, row, 0, 1, 2);
            ++row;
         };
         #define ADD(name) _add_button(QPalette::ColorRole::name, #name);
         ADD(NoRole);
         ADD(AlternateBase);
         ADD(Background);
         ADD(Base);
         ADD(Button);
         ADD(Highlight);
         ADD(Shadow);
         ADD(ToolTipBase);
         ADD(ToolTipText);
         ADD(Window);
         _add_header("Text");
         ADD(Text);
         ADD(BrightText);
         ADD(ButtonText);
         ADD(HighlightedText);
         ADD(PlaceholderText);
         ADD(WindowText);
         _add_header("Brightness roles");
         ADD(Light);
         ADD(Midlight);
         ADD(Mid);
         ADD(Dark);
         #undef ADD
      }

      auto _update_colors_from_group = [group_picker, buttons]() {
         auto palette = QApplication::palette();
         auto group   = (QPalette::ColorGroup) group_picker->currentData().toInt();
         for (const auto& pair : buttons) {
            auto  role   = pair.first;
            auto* button = pair.second;
            button->setColor(palette.color(group, role));
         }
      };
      _update_colors_from_group();
      QObject::connect(group_picker, qOverload<int>(&QComboBox::currentIndexChanged), dialog, _update_colors_from_group);
      
      dialog->show();
   }
}
