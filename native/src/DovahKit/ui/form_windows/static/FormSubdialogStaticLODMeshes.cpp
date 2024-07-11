#include "./FormSubdialogStaticLODMeshes.h"
#include <QMessageBox>
#include "dovah/forms/Static.h"

FormSubdialogStaticLODMeshes::FormSubdialogStaticLODMeshes(QWidget* parent) : QDialog(parent) {
   this->ui.setupUi(this);

   this->level_widgets = {
      LevelWidgets{
         .enabled   = nullptr,
         .model     = this->ui.lv0File,
         .propagate = this->ui.lv0Propagate,
      },
      LevelWidgets{
         .enabled   = this->ui.lv1Enable,
         .model     = this->ui.lv1File,
         .propagate = this->ui.lv1Propagate,
      },
      LevelWidgets{
         .enabled   = this->ui.lv2Enable,
         .model     = this->ui.lv2File,
         .propagate = this->ui.lv2Propagate,
      },
      LevelWidgets{
         .enabled   = this->ui.lv3Enable,
         .model     = this->ui.lv3File,
         .propagate = nullptr,
      },
   };
   for (size_t i = 0; i < this->level_widgets.size(); ++i) {
      auto& here = this->level_widgets[i];
      if (here.enabled) {
         QObject::connect(here.enabled, &QCheckBox::toggled, this, &FormSubdialogStaticLODMeshes::updateLevelStates);
      }

      here.model->setStandardConfiguration(DKGameFilePicker::StandardConfiguration::Meshes);
      QObject::connect(here.model, &DKGameFilePicker::pathChanged, this, [this, i, widget = here.model](QString path) {
         if (path.size() >= dovah::loaded_forms::Static::max_lod_mesh_path_length) {
            widget->setPath(this->level_widgets[i].last_good_path);
            QMessageBox::critical(
               this,
               tr("Error"),
               tr("The maximum path length is %1.").arg(dovah::loaded_forms::Static::max_lod_mesh_path_length)
            );
            return;
         }
         this->level_widgets[i].last_good_path = path;
         this->updateLevelStates();
      });

      if (here.propagate) {
         QObject::connect(here.propagate, &QPushButton::clicked, this, [this, i]() {
            this->propagateFrom(i);
         });
      }
   }
}

void FormSubdialogStaticLODMeshes::setPaths(const PathList& src) {
   bool enabled = true;
   for (size_t i = 0; i < path_count; ++i) {
      auto& widgets = this->level_widgets[i];
      const auto blockers = std::array{
         QSignalBlocker(widgets.enabled),
         QSignalBlocker(widgets.model)
      };

      if (!enabled) {
         if (widgets.enabled)
            widgets.enabled->setChecked(false);
         widgets.model->setPath("");
         continue;
      }
      if (src[i].empty()) {
         enabled = false;
         if (widgets.enabled)
            widgets.enabled->setChecked(false);
         widgets.model->setPath("");
      } else {
         if (widgets.enabled)
            widgets.enabled->setChecked(true);

         auto path = QString::fromStdString(src[i]);
         if (path.size() < dovah::loaded_forms::Static::max_lod_mesh_path_length) {
            widgets.last_good_path = path;
            widgets.model->setPath(path);
         }
      }
   }
   this->updateLevelStates();
}
FormSubdialogStaticLODMeshes::PathList FormSubdialogStaticLODMeshes::getPaths() const {
   PathList out;
   for (size_t i = 0; i < path_count; ++i) {
      auto& widgets = this->level_widgets[i];
      if (widgets.enabled && !widgets.enabled->isChecked())
         break;
      out[i] = widgets.model->path().toStdString();
   }
   return out;
}

void FormSubdialogStaticLODMeshes::propagateFrom(size_t src) {
   if (src >= this->level_widgets.size())
      return;

   auto src_path = this->level_widgets[src].model->path();
   for (size_t dst = src + 1; dst < this->level_widgets.size(); ++dst) {
      auto& here = this->level_widgets[dst];
      if (here.enabled)
         here.enabled->setChecked(true);
      here.model->setPath(src_path);
   }
}
void FormSubdialogStaticLODMeshes::updateLevelStates() {
   bool enabled = true;
   for (size_t i = 0; i < this->level_widgets.size(); ++i) {
      auto& here = this->level_widgets[i];

      const auto blockers = std::array{
         QSignalBlocker(here.enabled),
         QSignalBlocker(here.model)
      };

      if (enabled) {
         bool here_enabled = here.enabled == nullptr || !here.enabled->isChecked();
         bool here_empty   = here.model->path().isEmpty();

         if (!here_enabled) {
            here.model->setPath("");
            here.model->setEnabled(false);
         }
         if (!here_enabled || here_empty) {
            if (here.propagate)
               here.propagate->setEnabled(false);
         }

         enabled = here_enabled;
      } else {
         if (here.enabled) {
            here.enabled->setEnabled(false);
            here.enabled->setChecked(false);
         }
         here.model->setEnabled(false);
         here.model->setPath("");
         if (here.propagate)
            here.propagate->setEnabled(false);
      }
   }
}