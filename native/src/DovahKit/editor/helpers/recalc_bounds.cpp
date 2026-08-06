#include "./recalc_bounds.h"
#include <QMessageBox>
#include "dovah/files/bsa/bsa_archived_file.h"
#include "dovah/form_stub.h"
#include "dovah/forms/_component_access.h"
#include "dovah/forms/components/bounds.h"
#include "dovah/forms/components/model.h"
#include "dovah/utils/object_bounds_from_nif.h"
#include "editor/subsystems/assets.h"
#include "nif/block.h"
#include "nif/blocks/NiNode.h"
#include "nif/file.h"

namespace editor_helpers {
   extern void recalc_bounds(QWidget* parent_window, dovah::form_stub& stub) {
      auto loaded = stub.load();
      if (!loaded)
         return;
      auto* bounds = dovah::loaded_forms::component_access::get_object_bounds(loaded);
      if (!bounds)
         return;
      const auto* model = dovah::loaded_forms::component_access::get_model(loaded);
      if (!model)
         return;

      dovah::bsa_archived_file* file = nullptr;
      {
         auto path = std::filesystem::path("meshes") / std::filesystem::path(model->model_path);

         auto& am = dovahkit::subsystems::assets::get_or_create();
         file = am.lookup_game_asset(path, true);
      }
      if (!file)
         return;

      nifDK::file nif;
      nif.read((void*)file->data(), file->size());
      if (nif.read_error().empty()) {
         if (nif.root_node) {
            auto calc = dovah::utils::object_bounds_from_nif(*nif.root_node);
            if (!calc.is_undefined()) {
               auto _is_in_range = [](float v) {
                  return v >= -32768 && v <= 32767;
               };

               if (
                  _is_in_range(calc.min.x) &&
                  _is_in_range(calc.min.y) &&
                  _is_in_range(calc.min.z) &&
                  _is_in_range(calc.max.x) &&
                  _is_in_range(calc.max.y) &&
                  _is_in_range(calc.max.z)
               ) {
                  bounds->min.x = calc.min.x;
                  bounds->min.y = calc.min.y;
                  bounds->min.z = calc.min.z;
                  bounds->max.x = calc.max.x;
                  bounds->max.y = calc.max.y;
                  bounds->max.z = calc.max.z;
                  QMessageBox::information(
                     parent_window,
                     QObject::tr("Done"),
                     QObject::tr("Bounds recalculated.\n\nMin: (%1, %2, %3)\nMax: (%4, %5, %6)")
                        .arg(bounds->min.x)
                        .arg(bounds->min.y)
                        .arg(bounds->min.z)
                        .arg(bounds->max.x)
                        .arg(bounds->max.y)
                        .arg(bounds->max.z)
                  );
               } else {
                  QMessageBox::critical(parent_window, QObject::tr("Error"), QObject::tr("NIF is too large (bounds exceed [-32768, 32767] on at least one axis). Cannot apply new bounds."));
               }
            }
         } else {
            QMessageBox::critical(parent_window, QObject::tr("Error"), QObject::tr("NIF has no root node? Cannot recalc bounds."));
         }
      } else {
         QMessageBox::critical(parent_window, QObject::tr("Error"), QObject::tr("Unable to load this form's model. Cannot recalc bounds."));
      }
      delete file;
   }
}