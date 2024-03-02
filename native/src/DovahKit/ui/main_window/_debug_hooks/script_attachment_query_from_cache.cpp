#include "script_attachment_query_from_cache.h"
#include <QInputDialog>
#include "dovah/form_stub.h"
#include "editor/subsystems/papyrus/core.h"
#include "editor/core.h"

namespace DovahKitDebug::features {
   /*static*/ void script_attachment_query_from_cache::execute(QWidget* window) {
      dovah::bare_form_id_t form_id;
      std::string           scriptname;
      {
         auto text = QInputDialog::getText(window, QObject::tr("Form ID?", "debug"), QObject::tr("ID:", "debug"));
         if (text.isEmpty())
            return;
         form_id = text.toInt(nullptr, 16);
         if (!form_id)
            return;
      }
      {
         auto text = QInputDialog::getText(window, QObject::tr("Scriptname?", "debug"), QObject::tr("Name:", "debug"));
         if (text.isEmpty())
            return;
         scriptname = text.toStdString();
      }

      auto* stub = DovahKitCore::get().get_form(form_id);
      if (!stub) {
         __debugbreak(); // no stub
         return;
      }

      auto& papyrus = dovahkit::subsystems::papyrus::core::get();

      bool attached_to_form  = papyrus.form_has_script_attached(*stub, scriptname);
      bool attached_to_alias = false;
      if (stub->formType == dovah::form_type::quest) {
         attached_to_alias = papyrus.quest_has_script_attached_to_any_alias(*stub, scriptname);
      }
      __debugbreak();
   }
}