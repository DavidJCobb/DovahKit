#pragma once
#include <cstdint>
#include <map>
#include <vector>
#include <QObject>
#include <QPointer>
#include "helpers/eight_cc.h"
#include "helpers/singleton_ex.h"
#include "helpers/small_vector.h"
#include "dovah/form_types.h"

class AbstractFormEditDialog;
class DKFormListPane;
class DKHeaderView;

namespace dovahkit::subsystems::gui_adjust {
   class core;

   //
   // Subsystem for remembering user adjustments to things like list view column widths.
   // 
   // For now, serialization is not implemented; GUI adjustments are per-session only.
   //
   class core : public QObject, public cobb::singleton_ex<core> {
      Q_OBJECT;
      protected:
         core();
      public:
         using singleton_ex::get;
         using singleton_ex::get_or_create;

      protected:
         struct widget_prefs {
            cobb::eight_cc uid = 0;

            constexpr bool empty() const { return true; }
         };

         struct list_view_prefs : public widget_prefs {
            cobb::small_vector<int32_t, 8> widths; // stores "mod" factors for DKHeaderView

            constexpr bool empty() const { return widths.empty(); }
            void apply(DKHeaderView&) const;
            void store(DKHeaderView&);
         };

         struct dialog_prefs {
            union {
               dovah::form_type form_type;
               cobb::eight_cc   uid = 0; // for dialogs other than form-editing dialogs
            };
            cobb::small_vector<list_view_prefs, 2> listviews;
         };

         std::map<dovah::form_type, dialog_prefs> form_edit_dialogs;

         void _commit_list_view_prefs(dovah::form_type, const list_view_prefs&);

         void _onDialogClosed(dovah::form_type, cobb::eight_cc widget_id, DKHeaderView&);

      public:
         struct RegistrationRequest {
            cobb::eight_cc    id;
            QPointer<QWidget> widget = nullptr;
         };

         void registerWidget(AbstractFormEditDialog&, cobb::eight_cc widget_id, DKFormListPane&);
         void registerWidget(AbstractFormEditDialog&, cobb::eight_cc widget_id, DKHeaderView&);
         void registerWidgets(AbstractFormEditDialog&, std::vector<RegistrationRequest>&&);
   };
}