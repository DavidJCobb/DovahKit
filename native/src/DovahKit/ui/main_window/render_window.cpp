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
#include "editor/subsystems/worldedit.h"
#include "widgets/DKVulkanView.h"
#include "../../vulkan/rendered_light.h"
#include "../../vulkan/surface_renderer.h"

#include "editor/core.h"

#include "nif/file.h"
#include "nif/notice_code_t.h"
#include "nif/blocks/NiNode.h"
#include "nif/blocks/NiGeometry.h"
#include "nif/blocks/NiGeometryData.h"

#include <QBoxLayout>
#include <QDialog>
#include <QPushButton>
#include "dovah/files/bsa/bsa_archived_file.h"
#include "dovah/form_stub.h"
#include "dovah/form_stub_helpers.h"
#include "dovah/forms/Cell.h"
#include "dovah/forms/Form.h"
#include "dovah/forms/ObjectReference.h"
#include "dovah/forms/components/extra_data/scale.h"
#include "dovah/forms/components/model.h"
#include "ui/generic/FormPicker.h"
#include "vulkan/helpers/glm_transform_from_beth.h"

namespace {
   static constexpr bool use_new_renderer = true;
}

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
   {
      auto& worldedit = dovahkit::subsystems::worldedit::get_or_create();
      worldedit.set_target_view(*view);
      QObject::connect(&worldedit, &dovahkit::subsystems::worldedit::statusBarMessage, this, [this](const QString& message, int timeout) {
         this->status->showMessage(message, timeout);
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

   for (size_t i = 0; i < 3; ++i) {
      auto* button = new QToolButton(this->toolbar);
      button->setText(QString("Pause #%1").arg(i));
      button->setCheckable(true);
      QObject::connect(button, &QAbstractButton::toggled, this, [this, view, i](bool checked) {
         view->surfaceRenderer()->set_animation_paused(i, checked);
      });
      button->setIcon(this->style()->standardIcon(QStyle::SP_MediaPause));
      //
      this->toolbar->addWidget(button);
   }
   //
   {
      auto* button = new QToolButton(this->toolbar);
      button->setText("New Object");
      QObject::connect(button, &QAbstractButton::clicked, this, [this, view]() {
         auto path = QFileDialog::getOpenFileName(this, "Texture file", "", "Image (*.dds, *.png, *.bmp)");
         if (path.isEmpty())
            return;
         view->surfaceRenderer()->add_mesh(path);
      });
      button->setIcon(this->style()->standardIcon(QStyle::SP_FileDialogNewFolder));
      //
      this->toolbar->addWidget(button);
   }
   {
      auto* button = new QToolButton(this->toolbar);
      button->setText("Delete Last Object");
      QObject::connect(button, &QAbstractButton::clicked, this, [this, view]() {
         view->surfaceRenderer()->remove_last_mesh();
      });
      button->setIcon(this->style()->standardIcon(QStyle::SP_BrowserStop));
      //
      this->toolbar->addWidget(button);
   }
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
   //
   {
      auto* button = new QToolButton(this->toolbar);
      button->setText("Load NIF...");
      QObject::connect(button, &QAbstractButton::clicked, this, [this, view]() {
         if (!DovahKitCore::get().has_data()) {
            QMessageBox::critical(this, "Error", "Load data first, so we know where to pull game textures from.");
            return;
         }
         auto path = QFileDialog::getOpenFileName(this, "Model", "", "NetImmerse Format model (*.nif)");
         if (path.isEmpty())
            return;
         nifDK::file model;
         {
            QFile file(path);
            if (!file.open(QIODevice::ReadOnly)) {
               QMessageBox::critical(this, "Error", "Failed to open NIF file.");
               return;
            }
            QByteArray data = file.readAll();
            model.read((void*)data.constData(), data.size());
            //
            auto& error = model.read_error();
            if (error.code != nifDK::default_notice_code) {
               qDebug("NIF parsing failed with error code %08X.", error.code);
               QMessageBox::critical(this, "Error", QString("NIF parsing failed with error code %1").arg(error.code, 8, 16, QChar('0')));
               #if _DEBUG
                  __debugbreak();
               #endif
               return;
            }
         }
         qDebug("NIF parsed. Passing to surface_renderer...");
         view->surfaceRenderer()->add_nif(model);
      });
      button->setIcon(this->style()->standardIcon(QStyle::SP_FileIcon));
      //
      this->toolbar->addWidget(button);
   }
   //
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
            nifDK::file model;
            {
               std::filesystem::path path = std::string("meshes") + (form_model->model_path[0] == '/' || form_model->model_path[0] == '\\' ? "" : "\\") + form_model->model_path;
               std::unique_ptr<dovah::bsa_archived_file> file(DovahKitCore::get().lookup_game_asset(path));
               if (!file) {
                  QMessageBox::critical(this, "Error", "Failed to open NIF file.");
                  return;
               }
               model.read((void*)file->data(), file->size());
               //
               auto& error = model.read_error();
               if (error.code != nifDK::default_notice_code) {
                  qDebug("NIF parsing failed with error code %08X.", error.code);
                  QMessageBox::critical(this, "Error", QString("NIF parsing failed with error code %1").arg(error.code, 8, 16, QChar('0')));
                  #if _DEBUG
                     __debugbreak();
                  #endif
                  return;
               }
            }
            qDebug("NIF parsed. Passing to surface_renderer...");
            view->surfaceRenderer()->add_nif(model);
         });
      });
      button->setIcon(this->style()->standardIcon(QStyle::SP_FileIcon));
      //
      this->toolbar->addWidget(button);
   }
   //
   {
      auto* button = new QToolButton(this->toolbar);
      button->setText("Import cell...");
      QObject::connect(button, &QAbstractButton::clicked, this, [this, view]() {
         auto& editor = DovahKitCore::get();
         if (!editor.has_data()) {
            QMessageBox::critical(this, "Error", "Load data first, so we have base forms to import.");
            return;
         }
         auto input = QInputDialog::getText(this, "Select cell", "Cell editor ID or form ID");
         if (input.isEmpty())
            return;
         dovah::bare_form_id_t formID = input.toUInt(nullptr, 16);
         if (!formID) {
            QMessageBox::critical(this, "Error", "Input was zero or not a form ID.");
            return;
         }
         //
         auto* cell = editor.get_form_of_probable_type(dovah::form_type::cell, formID);
         if (!cell) {
            QMessageBox::critical(this, "Error", "Form doesn't exist.");
            return;
         }
         if (cell->formType != dovah::form_type::cell) {
            QMessageBox::critical(this, "Error", "Form is not a cell.");
            return;
         }
         //
         auto* sr = view->surfaceRenderer();
         dovah::form_stub_helpers::for_each_child_form(cell, [this, sr](dovah::form_stub* stub) {
            if (stub->formType != dovah::form_type::reference)
               return false;
            auto* base = dovah::form_stub_helpers::get_base_form(stub);
            if (!base)
               return false;
            //
            if (base->formType == dovah::form_type::light) {
               auto loaded = stub->load().ptr_cast<dovah::loaded_forms::ObjectReference>();
               if (!loaded)
                  return false;
               sr->add_light(*loaded);
               return false;
            }
            //
            auto loaded_base = base->load();
            if (!loaded_base)
               return false;
            auto* form_model = loaded_base->get_model();
            if (!form_model || form_model->model_path.empty())
               return false;
            //
            auto loaded = stub->load().ptr_cast<dovah::loaded_forms::ObjectReference>();
            if (!loaded)
               return false;
            //
            float scale = loaded->get_scale();
            //
            nifDK::file model;
            {
               std::filesystem::path path = std::string("meshes") + (form_model->model_path[0] == '/' || form_model->model_path[0] == '\\' ? "" : "\\") + form_model->model_path;
               std::unique_ptr<dovah::bsa_archived_file> file(DovahKitCore::get().lookup_game_asset(path));
               if (!file) {
                  qDebug("Failed to open NIF file: <%s>", path.string().c_str());
                  return false;
               }
               model.read((void*)file->data(), file->size());
               //
               auto& error = model.read_error();
               if (error.code != nifDK::default_notice_code) {
                  qDebug("Failed to parse NIF file: <%s>\n - Error code %08X.", path.string().c_str(), error.code);
                  #if _DEBUG
                     __debugbreak();
                  #endif
                  return false;
               }
            }
            qDebug("NIF parsed. Passing to surface_renderer...");
            sr->add_nif(
               model,
               glm::vec3{ loaded->position.x, loaded->position.y, loaded->position.z },
               glm::vec3{ loaded->rotation.x, loaded->rotation.y, loaded->rotation.z },
               scale
            );
            //
            return false;
         });
         {
            auto _to_vec = [](const dovah::loaded_forms::color_t& color) {
               return glm::vec3{ (float)color.r / 255.0F, (float)color.g / 255.0F, (float)color.b / 255.0F };
            };
            //
            auto  loaded = cell->load().ptr_cast<dovah::loaded_forms::Cell>();
            auto& sgs    = sr->scene.global_state;
            {
               auto& lt = loaded->interior.lighting;
               sgs.ambient_light_color = _to_vec(lt.ambient);
               sgs.sun_color      = _to_vec(lt.directional);
               // TODO: sgs.sun_dir
               sgs.fog_color_near = _to_vec(lt.fog_color_near);
               sgs.fog_color_far  = _to_vec(lt.fog_color_far);
               sgs.fog_plane_near = lt.fog_distance_near;
               sgs.fog_plane_far  = lt.fog_distance_far;
               sgs.fog_power      = lt.fog_power;
               sgs.fog_max        = lt.fog_max;
               sgs.interior_clip_distance = lt.fog_distance_clip;
            }
            if (loaded->interior.lighting_template) {
               // TODO: load the LTMP and use its params
               //       for now, we just reset some fields to safe defaults
               //sgs.fog_max = 0;
               sgs.interior_clip_distance = 0;
            }
         }
      });
      button->setIcon(this->style()->standardIcon(QStyle::SP_FileIcon));
      //
      this->toolbar->addWidget(button);
   }
   //
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
      auto* button = new QToolButton(this->toolbar);
      button->setText("Spawn shadow-caster debug scene");
      QObject::connect(button, &QAbstractButton::clicked, this, [this, view]() {
         auto* sr = view->surfaceRenderer();
         //
         nifDK::file model;
         {
            std::unique_ptr<dovah::bsa_archived_file> file(DovahKitCore::get().lookup_game_asset("meshes/dungeons/genkit/genkitrmcorin01.nif"));
            if (!file) {
               qDebug("Failed to open NIF file");
               return;
            }
            model.read((void*)file->data(), file->size());
            //
            auto& error = model.read_error();
            if (error.code != nifDK::default_notice_code) {
               qDebug("Failed to parse NIF file\n - Error code %08X.", error.code);
               #if _DEBUG
                  __debugbreak();
               #endif
               return;
            }
         }
         constexpr auto model_size = 256.0F;
         for (int j = 0; j < 2; ++j) {
            for (int i = 0; i < 4; ++i) {
               constexpr auto rotations = std::array{ 270, 180, 0, 90 };
               auto x = model_size * (i % 2) - (model_size / 2);
               auto y = model_size * (i / 2) - (model_size / 2);
               if (j) {
                  x *= 2;
                  y *= 2;
               }
               sr->add_nif(
                  model,
                  glm::vec3{ x, y, j ? 1.0F : ((model_size / 2) + 1.0F) },
                  glm::vec3{ 0, 0, glm::radians<float>(rotations[i]) },
                  j ? 2.0 : 1.0
               );
            }
         }
         //
         sr->add_light({
            .transform = vulkanDK::glm_transform_from_beth(glm::fvec3{ 0, 0, 256 }, glm::fvec3{ 0, 0, 0 }, 1.0F),
            .color     = { 1.0F, 0.2F, 0.2F },
            .fade      = 1.0F,
            .fov       = glm::radians(90.0F),
            .radius    = 1024.0F,
            .type      = vulkanDK::rendered_light::light_type::spot_shadow,
         });
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
   //
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
   //
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
}