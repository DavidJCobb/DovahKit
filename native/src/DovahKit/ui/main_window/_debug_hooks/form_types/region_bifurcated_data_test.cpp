#include "./region_bifurcated_data_test.h"
#include <QMessageBox>

#include "dovah/forms/Region.h"
#include "dovah/forms/Static.h"
#include "editor/core.h"

namespace DovahKitDebug::features::form_types {
   /*static*/ void region_bifurcated_data_test::execute(QWidget* from) {
      auto& editor = DovahKitCore::get();
      if (!editor.has_data()) {
         QMessageBox::critical(
            from,
            "Error",
            QString("No data loaded. Do a data load first, even if it's with zero files selected.")
         );
         return;
      }

      auto* statik_a = editor.create_form_of_type(dovah::form_type::statik);
      auto* statik_b = editor.create_form_of_type(dovah::form_type::statik);
      auto* region   = editor.create_form_of_type(dovah::form_type::region);

      emit editor.formModificationImminent(statik_a);
      statik_a->editorID = "aaaTESTStaticA";
      statik_a->set_edited(true);
      emit editor.formModified(statik_a);

      emit editor.formModificationImminent(statik_b);
      statik_b->editorID = "aaaTESTStaticB";
      statik_b->set_edited(true);
      emit editor.formModified(statik_b);

      emit editor.formModificationImminent(region);
      region->editorID   = "aaaTESTRegion";
      //
      auto loaded = region->load().ptr_cast<dovah::loaded_forms::Region>();
      assert(!!loaded);
      //
      loaded->generable_content.emplace_back();
      loaded->generable_content.emplace_back();
      auto& content_a = loaded->generable_content[0];
      auto& content_b = loaded->generable_content[1];
      //
      auto& objects_a = content_a.get_or_emplace<dovah::loaded_forms::structs::region::generable_content::raw_object_collection>(*loaded);
      auto& objects_b = content_b.get_or_emplace<dovah::loaded_forms::structs::region::generable_content::raw_object_collection>(*loaded);
      //
      {
         auto& item = objects_a.objects.emplace_back();
         item.form.set(*loaded, statik_a);
      }
      {
         auto& item = objects_b.objects.emplace_back();
         item.form.set(*loaded, statik_b);
      }
      //
      region->set_edited(true);
      emit editor.formModified(region);

      QMessageBox::information(
         from,
         "Done",
         QString("Created: aaaTESTRegion, aaaTESTStaticA, aaaTESTStaticB.")
      );
   }
}