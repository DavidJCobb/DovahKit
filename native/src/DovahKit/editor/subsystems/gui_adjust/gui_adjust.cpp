#include "gui_adjust.h"
#include <QDialog>
#include <QPointer>
#include "ui/form_windows/_base.h"
#include "widgets/DKFormListPane.h"
#include "widgets/DKHeaderView.h"

namespace dovahkit::subsystems::gui_adjust {
   core::core() {
   }

   void core::list_view_prefs::apply(DKHeaderView& widget) const {
      int colCount = std::min((int)this->widths.size(), widget.count());
      for (int i = 0; i < colCount; ++i) {
         widget.setColumnModFactor(i, this->widths[i]);
      }
      if (colCount > 0)
         widget.reapplyColumnFlex();
   }
   void core::list_view_prefs::store(DKHeaderView& widget) {
      int colCount = widget.count();
      for (int i = 0; i < colCount; ++i) {
         int m = widget.columnModFactor(i);
         if (m > 5 || m < -5) {
            this->widths.resize(i + 1);
            this->widths[i] = m;
         }
      }
   }

   void core::_commit_list_view_prefs(dovah::form_type ft, const list_view_prefs& prefs) {
      auto& item = this->form_edit_dialogs[ft];
      item.form_type = ft;

      for (auto& widget : item.listviews) {
         if (widget.uid == prefs.uid) {
            widget = prefs;
            return;
         }
      }
      if (prefs.empty())
         return;
      item.listviews.push_back(prefs);
   }

   void core::registerWidget(AbstractFormEditDialog& dialog, cobb::eight_cc widget_id, DKFormListPane& widget) {
      auto* hv = widget.horizontalHeader();
      if (!hv)
         return;
      if (auto* dkhv = dynamic_cast<DKHeaderView*>(hv))
         return this->registerWidget(dialog, widget_id, *dkhv);
      qDebug("[gui_adjust] Failed to register widget...");
   }
   void core::registerWidget(AbstractFormEditDialog& dialog, cobb::eight_cc widget_id, DKHeaderView& hv) {
      auto ft = dialog.formType();

      QPointer phv = &hv; // in case the header view is deleted for some reason before its window closes
      QObject::connect(&dialog, &QDialog::finished, this, [this, ft, widget_id, phv](int result) {
         list_view_prefs prefs;
         prefs.uid = widget_id;
         prefs.store(*phv);
         this->_commit_list_view_prefs(ft, prefs);
      });

      //
      // Look up and apply any already-stored prefs:
      //
      auto it = this->form_edit_dialogs.find(ft);
      if (it != this->form_edit_dialogs.end()) {
         const auto& prefs = it->second;
         for (const auto& item : prefs.listviews) {
            if (item.uid != widget_id)
               continue;
            item.apply(hv);
         }
      }
   }
   void core::registerWidgets(AbstractFormEditDialog& dialog, std::vector<RegistrationRequest>&& entries) {
      auto ft = dialog.formType();

      //
      // Look up and apply any already-stored prefs:
      //
      auto it = this->form_edit_dialogs.find(ft);
      if (it != this->form_edit_dialogs.end()) {
         const auto& prefs = it->second;
         for (const auto& item : prefs.listviews) {
            QWidget* target = nullptr;
            for (const auto& entry : entries) {
               if (entry.id == item.uid) {
                  target = entry.widget;
                  break;
               }
            }
            if (!target)
               break;

            DKHeaderView* dkhv = nullptr;
            if (auto* casted = dynamic_cast<DKFormListPane*>(target)) {
               if (auto* header = dynamic_cast<DKHeaderView*>(casted->horizontalHeader())) {
                  dkhv = header;
               }
            } else if (auto* casted = dynamic_cast<DKHeaderView*>(target)) {
               dkhv = casted;
            }
            if (dkhv) {
               item.apply(*dkhv);
            }
         }
      }

      //
      // Store changes in prefs once the dialog is closed:
      //
      QObject::connect(&dialog, &QDialog::finished, this, [this, ft, list = std::move(entries)](int result) {
         for (const auto& item : list) {
            if (!item.widget)
               continue;

            //
            // Identify type of widget:
            //

            DKHeaderView* dkhv = nullptr;

            if (auto* widget = dynamic_cast<DKFormListPane*>(item.widget.data())) {
               if (auto* header = dynamic_cast<DKHeaderView*>(widget->horizontalHeader())) {
                  dkhv = header;
               }
            } else if (auto* widget = dynamic_cast<DKHeaderView*>(item.widget.data())) {
               dkhv = widget;
            }

            //
            // Store widget adjustments:
            //

            if (dkhv) {
               list_view_prefs prefs;
               prefs.uid = item.id;
               prefs.store(*dkhv);
               this->_commit_list_view_prefs(ft, prefs);
            }
         }
      });
   }
}
