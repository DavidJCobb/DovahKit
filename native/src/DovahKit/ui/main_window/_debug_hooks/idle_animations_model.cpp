#include "idle_animations_model.h"
#include <QDialog>
#include <QGridLayout>
#include <QTreeView>
#include "dovah/form_stub.h"
#include "ui/form_windows/idle/IdleAnimationFormsModel.h"
#include "ui/form_windows/idle/IdleAnimationFormsModel_2.h"

#include "editor/core.h"
#include <QMenu>
#include <QAction>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include "widgets/DKFormPicker.h"
#include "ui/form_windows/idle/IdleNewActionRootPickerFilter.h"
#include "ui/utils/set_custom_context_menu.h"
#include "dovah/forms/IdleAnimation.h"

namespace DovahKitDebug::features {
   /*static*/ void idle_animations_model::execute(QWidget* from) {
      auto* dialog = new QDialog(from);
      auto* layout = new QBoxLayout(QBoxLayout::Direction::Down, dialog);
      auto* widget = new QTreeView(dialog);
      QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);
      
      auto* model = new IdleAnimationFormsModel_2(widget);
      widget->setModel(model);
      widget->setHeaderHidden(true);

      layout->addWidget(widget);

      {
         auto* menu = new QMenu(widget);
         ui::set_custom_context_menu(*widget, *menu);
         {
            auto* action = new QAction(QObject::tr("Add action"), menu);
            menu->addAction(action);
            QObject::connect(action, &QAction::triggered, widget, [dialog, widget, model]() {
               auto* sel_model = widget->selectionModel();
               auto  rows      = sel_model->selectedRows();
               if (rows.empty())
                  return;
               auto qmi       = rows[0];
               auto node_type = model->data(qmi, IdleAnimationFormsModel_2::NodeTypeRole).value<IdleAnimationFormsModel_2::NodeType>();
               if (node_type != IdleAnimationFormsModel_2::NodeType::Graph)
                  return;

               dovah::form_stub* actionStub = nullptr;
               QString idleEditorID;
               {
                  QDialog modal(dialog);
                  modal.setModal(true);

                  auto* layout = new QGridLayout(&modal);
                  auto* picker = new DKFormPicker(&modal);
                  auto* ed_id  = new QLineEdit(&modal);
                  {
                     auto* label  = new QLabel(QObject::tr("Action:"));
                     auto* filter = new IdleNewActionRootPickerFilter(picker);
                     filter->actions_to_exclude = model->actionsByGraph(qmi);
                     label->setBuddy(picker);
                     picker->setAllowedFormType(dovah::form_type::action);
                     picker->setCustomFilter(filter);
                     picker->setAllowNone(false);
                     layout->addWidget(label,  0, 0);
                     layout->addWidget(picker, 0, 1);
                  }
                  {
                     auto* label  = new QLabel(QObject::tr("Idle Editor ID:"));
                     label->setBuddy(ed_id);
                     layout->addWidget(label, 1, 0);
                     layout->addWidget(ed_id, 1, 1);
                  }
                  {
                     auto* buttons_layout = new QHBoxLayout();
                     layout->addLayout(buttons_layout, 2, 0, 1, 2);
                     buttons_layout->addStretch(1);
                     {
                        auto* button = new QPushButton(QObject::tr("OK"));
                        buttons_layout->addWidget(button);
                        QObject::connect(button, &QPushButton::clicked, &modal, &QDialog::accept);

                        QObject::connect(ed_id, &QLineEdit::textEdited, button, [button](QString text) {
                           button->setEnabled(!text.isEmpty());
                        });
                     }
                     {
                        auto* button = new QPushButton(QObject::tr("Cancel"));
                        buttons_layout->addWidget(button);
                        QObject::connect(button, &QPushButton::clicked, &modal, &QDialog::reject);
                     }
                     buttons_layout->addStretch(1);
                  }

                  if (modal.exec() == QDialog::Rejected)
                     return;

                  actionStub = picker->formStub();
                  if (!actionStub)
                     return;
                  idleEditorID = ed_id->text();
               }
               model->createActionRoot(qmi, *actionStub, idleEditorID);
            });
         }
      }
      
      dialog->show();
   }
}
