#include "render_window.h"
#include "helpers/rotation.h"
#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QStatusBar>
#include <QStyle>
#include <QToolButton>
#include "editor/subsystems/worldedit/core.h"
#include "widgets/DKVulkanView.h"
#include "vulkan/rendered_light.h"
#include "vulkan/surface_renderer.h"

#include "editor/core.h"

#include "nif/file.h"
#include "nif/notice_code_t.h"
#include "nif/blocks/NiNode.h"
#include "nif/blocks/NiGeometry.h"
#include "nif/blocks/NiGeometryData.h"

#include <QBoxLayout>
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include "dovah/files/bsa/bsa_archived_file.h"
#include "dovah/form_stub.h"
#include "dovah/form_stub_helpers.h"
#include "dovah/forms/Form.h"
#include "dovah/forms/components/model.h"
#include "editor/helpers/form_identifiers_to_string.h"
#include "ui/generic/FormPicker.h"
#include "vulkan/helpers/glm_transform_from_beth.h"
#include "vulkan/enums/gizmo_mode.h"

#include "helpers/math/rotation/unit_conversion.h"

RenderWindow::RenderWindow(QWidget* parent) : QWidget(parent) {
   this->setWindowTitle(tr("Render Window"));
   this->setMinimumSize({ 150, 150 });
   //
   auto* layout = new QVBoxLayout(this);
   layout->setContentsMargins({ 0, 0, 0, 0 });
   //
   DKVulkanView* view = new DKVulkanView(this);
   view->setInputHandlingEnabled(true);
   //
   layout->addWidget(view, 1);
   view->setFocus(); // default focus within this window
   {
      auto& worldedit = dovahkit::subsystems::worldedit::core::get_or_create();
      worldedit.set_target_view(*view);
      QObject::connect(&worldedit, &dovahkit::subsystems::worldedit::core::refSelectionChanged, this, [this](dovah::form_stub& refr, bool selected) {
         auto* base = dovah::form_stub_helpers::get_base_form(&refr);
         if (!base)
            return;

         QString message;
         if (selected) {
            message = tr("Selected form %1 (base %2).");
         } else {
            message = tr("Deselected form %1 (base %2).");
         }
         this->status->showMessage(
            message
               .arg(editor_helpers::form_identifiers_to_string(&refr))
               .arg(editor_helpers::form_identifiers_to_string(base)),
            3000
         );
      });
   }
   //
   this->toolbar = new QToolBar(this);
   layout->setMenuBar(this->toolbar);
   {
      auto* sb = new QStatusBar(this);
      this->status = sb;
      layout->addWidget(sb, 0);
   }

   // camera jump to coords
   {
      auto* button = new QToolButton(this->toolbar);
      button->setText("Set Camera Position");
      QObject::connect(button, &QAbstractButton::clicked, this, [this, view]() {
         glm::fvec3 pos;
         bool ok;
         //
         auto& camera = view->surfaceRenderer()->scene.camera;
         //
         pos.x = QInputDialog::getDouble(this, "X", "Input X-coordinate", camera.position.x, -999999, 999999, 4, &ok);
         if (!ok)
            return;
         pos.y = QInputDialog::getDouble(this, "Y", "Input Y-coordinate", camera.position.y, -999999, 999999, 4, &ok);
         if (!ok)
            return;
         pos.z = QInputDialog::getDouble(this, "Z", "Input Z-coordinate", camera.position.z, -999999, 999999, 4, &ok);
         if (!ok)
            return;
         //
         view->surfaceRenderer()->set_camera_position(pos);
      });
      button->setIcon(this->style()->standardIcon(QStyle::SP_VistaShield));
      //
      this->toolbar->addWidget(button);
   }
   {
      auto* button = new QToolButton(this->toolbar);
      button->setText("Set Camera Rotation");
      QObject::connect(button, &QAbstractButton::clicked, this, [this, view]() {
         glm::fvec3 pos;
         bool ok;
         //
         auto& scene  = view->surfaceRenderer()->scene;
         auto& camera = scene.camera;
         //
         pos.x = QInputDialog::getDouble(this, "X", "Input X-rotation (pitch)", cobb::radians_to_degrees(camera.pitch), -999999, 999999, 4, &ok);
         if (!ok)
            return;
         pos.y = QInputDialog::getDouble(this, "Y", "Input Y-rotation (roll)", cobb::radians_to_degrees(camera.roll), -999999, 999999, 4, &ok);
         if (!ok)
            return;
         pos.z = QInputDialog::getDouble(this, "Z", "Input Z-rotation (yaw)", cobb::radians_to_degrees(camera.yaw), -999999, 999999, 4, &ok);
         if (!ok)
            return;
         for (int i = 0; i < 3; ++i)
            pos[i] = cobb::degrees_to_radians(pos[i]);
         //
         camera.pitch = pos.x;
         camera.roll  = pos.y;
         camera.yaw   = pos.z;
         scene.update_camera();
      });
      button->setIcon(this->style()->standardIcon(QStyle::SP_VistaShield));
      //
      this->toolbar->addWidget(button);
   }
   
   {
      auto* button = new QToolButton(this->toolbar);
      button->setText("Import base form...");
      QObject::connect(button, &QAbstractButton::clicked, this, [this, view]() {
         if (!DovahKitCore::get().has_data()) {
            QMessageBox::critical(this, "Error", "Load data first, so we have base forms to import.");
            return;
         }
         auto* dialog = new QDialog(this);
         auto* layout = new QVBoxLayout(dialog);
         auto* picker = new FormPicker(dialog);
         auto* button = new QPushButton("Import", dialog);
         layout->addWidget(picker);
         layout->addWidget(button);
         //picker->setAllowNone(false);
         picker->setMinimumWidth(400);
         {
            QVector<dovah::form_type_t> types;
            for (auto& ft : dovah::form_types) {
               if (dovah::form_type_info::form_type_is_base_form(ft.formType))
                  types.push_back(ft.formType);
            }
            picker->setAllowedFormTypes(types);
         }
         //
         dialog->show();
         QObject::connect(button, &QPushButton::clicked, this, [view, this, dialog, picker]() {
            auto* form = picker->formStub();
            dialog->accept();
            //
            if (!form)
               return;
            auto loaded = form->load();
            if (!loaded) {
               QMessageBox::critical(this, "Error", "Unable to load this form type.");
               return;
            }
            auto* form_model = loaded->get_model();
            if (!form_model) {
               QMessageBox::critical(this, "Error", "This base form type has no model.");
               return;
            }
            if (form_model->model_path.empty()) {
               QMessageBox::critical(this, "Error", "No model path set for this base form.");
               return;
            }
            view->surfaceRenderer()->add_nif(*form, *form_model);
         });
      });
      button->setIcon(this->style()->standardIcon(QStyle::SP_FileIcon));
      //
      this->toolbar->addWidget(button);
   }
   
   {
      auto* button = new QToolButton(this->toolbar);
      button->setText("Debug frustrums");
      QObject::connect(button, &QAbstractButton::clicked, this, [this, view]() {
         view->surfaceRenderer()->debug_show_frustrums();
      });
      button->setIcon(this->style()->standardIcon(QStyle::SP_DriveCDIcon));
      //
      this->toolbar->addWidget(button);
   }
   //
   {
      auto* widget = new QComboBox(this->toolbar);
      widget->addItem("Normal", -1);
      widget->addItem("Shadow Caster 0", 0);
      widget->addItem("Shadow Caster 1", 1);
      widget->addItem("Shadow Caster 2", 2);
      widget->addItem("Shadow Caster 3", 3);
      QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, view, widget]() {
         auto* sr = view->surfaceRenderer();
         auto  i  = widget->currentData().toInt();
         //
         if (i < 0)
            sr->debug_show_shadow_caster_culling(std::string::npos);
         else
            sr->debug_show_shadow_caster_culling(i);
      });
      this->toolbar->addWidget(widget);
   }
   //
   {
      auto* widget = new QCheckBox("Freeze culling updates", this->toolbar);
      QObject::connect(widget, &QCheckBox::toggled, this, [this, view](bool checked) {
         auto* sr = view->surfaceRenderer();
         sr->debug_set_culling_updates_frozen(checked);
      });
      this->toolbar->addWidget(widget);
   }
   
   {
      auto* widget = new QCheckBox("Landscape wire", this->toolbar);
      QObject::connect(widget, &QCheckBox::toggled, this, [this, view](bool checked) {
         auto* sr = view->surfaceRenderer();
         sr->debug_set_landscape_wireframes_visible(checked);
      });
      this->toolbar->addWidget(widget);
   }
   //
   {
      auto* widget = new QCheckBox("Landscape normals", this->toolbar);
      QObject::connect(widget, &QCheckBox::toggled, this, [this, view](bool checked) {
         auto* sr = view->surfaceRenderer();
         sr->debug_set_landscape_normals_visible(checked);
      });
      this->toolbar->addWidget(widget);
   }
   
   {
      auto* button = new QToolButton(this->toolbar);
      button->setText("Debugbreak on next draw");
      QObject::connect(button, &QAbstractButton::clicked, this, [this, view]() {
         view->surfaceRenderer()->debug_break_on_next_draw();
      });
      button->setIcon(this->style()->standardIcon(QStyle::SP_MediaPause));
      //
      this->toolbar->addWidget(button);
   }
   
   {
      auto* button = new QToolButton(this->toolbar);
      button->setText("Land borders");
      button->setCheckable(true);
      QObject::connect(button, &QAbstractButton::clicked, this, [view, button]() {
         view->surfaceRenderer()->set_landscape_borders_visible(button->isChecked());
      });
      //
      this->toolbar->addWidget(button);
   }
   
   // gizmo tests
   {
      auto* button = new QToolButton(this->toolbar);
      button->setText("Gizmo test");
      button->setCheckable(true);
      QObject::connect(button, &QAbstractButton::clicked, this, [view, button]() {
         auto* sr = view->surfaceRenderer();
         sr->set_gizmo_mode(button->isChecked() ? vulkanDK::gizmo_mode::translate : vulkanDK::gizmo_mode::none);
      });
      //
      this->toolbar->addWidget(button);
   }
   {
      auto* button = new QToolButton(this->toolbar);
      button->setText("Gizmo test highlight");
      button->setCheckable(true);
      QObject::connect(button, &QAbstractButton::clicked, this, [view, button]() {
         auto* sr = view->surfaceRenderer();
         sr->set_gizmo_axis_highlighted(vulkanDK::axis3D::y, button->isChecked());
      });
      //
      this->toolbar->addWidget(button);
   }
   {
      auto* widget = new QComboBox(this->toolbar);
      widget->addItem("Translate", (int)vulkanDK::gizmo_mode::translate);
      widget->addItem("Rotate", (int)vulkanDK::gizmo_mode::rotate);
      widget->addItem("Scale", (int)vulkanDK::gizmo_mode::scale);
      QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, view, widget]() {
         auto* sr = view->surfaceRenderer();
         auto  i  = widget->currentData().toInt();
         sr->set_gizmo_mode((vulkanDK::gizmo_mode)i);
      });
      this->toolbar->addWidget(widget);
   }

   {
      auto* button = new QToolButton(this->toolbar);
      button->setText("Move Selection");
      QObject::connect(button, &QAbstractButton::clicked, this, [this, view]() {
         auto& worldedit = dovahkit::subsystems::worldedit::core::get_or_create();
         if (worldedit.get_selected_refs().empty()) {
            QMessageBox::warning(this, "Error", "No refs selected");
            return;
         }

         cobb::vector3<float> pos;
         cobb::vector3<float> rot;
         bool ok;
         //
         pos.x = QInputDialog::getDouble(this, "X", "Translate X", 0, -999999, 999999, 4, &ok);
         if (!ok)
            return;
         pos.y = QInputDialog::getDouble(this, "Y", "Translate Y", 0, -999999, 999999, 4, &ok);
         if (!ok)
            return;
         pos.z = QInputDialog::getDouble(this, "Z", "Translate Z", 0, -999999, 999999, 4, &ok);
         if (!ok)
            return;
         //
         rot.x = QInputDialog::getDouble(this, "X", "Rotate X", 0, -360, 360, 4, &ok);
         if (!ok)
            return;
         rot.y = QInputDialog::getDouble(this, "Y", "Rotate Y", 0, -360, 360, 4, &ok);
         if (!ok)
            return;
         rot.z = QInputDialog::getDouble(this, "Z", "Rotate Z", 0, -360, 360, 4, &ok);
         if (!ok)
            return;

         rot *= cobb::degrees_to_radians_mult;
         
         std::decay_t<decltype(worldedit)>::coordinate_adjustment adjustment;
         adjustment.pos = pos;
         adjustment.rot = rot;

         bool result = worldedit.try_adjust_selection_coordinates(adjustment);
         if (!result) {
            QMessageBox::warning(this, "Error", "Adjustment failed");
            return;
         }
      });
      button->setIcon(this->style()->standardIcon(QStyle::SP_VistaShield));
      //
      this->toolbar->addWidget(button);
   }
}