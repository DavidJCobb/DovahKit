#include "textureset.h"
#include "_base_cpp.h"
#include "../../helpers/bitwise.h"

#include "../../incomplete_code_warnings.h"
static_assert(incomplete_code_warnings::allow_compiling_despite_incomplete_form_dialogs, "The form-editing dialog for TextureSets is incomplete: the preview pane is not yet functional.");

namespace {
   using form_type  = dovah::loaded_forms::TextureSet;
   using decal_type = dovah::loaded_forms::components::decal_data;

   constexpr bool hide_inapplicable_paths = true;
}

FormDialogTextureSet::FormDialogTextureSet(dovah::form_stub* stub, QWidget* parent) : FormDialogBaseTemplate(stub, parent) {
   form_dialog_helpers::initialize<FormDialogTextureSet, dovah::loaded_forms::TextureSet>(*this, stub);
   //
   QObject::connect(this->ui.flagSpecular, &QCheckBox::toggled, this, [this](bool checked) {
      cobb::edit_bit(this->form->texture_flags, form_type::texture_set_flag::no_specular_map, !checked);
   });
   QObject::connect(this->ui.flagSkinTexture, &QCheckBox::toggled, this, [this](bool checked) {
      cobb::edit_bit(this->form->texture_flags, form_type::texture_set_flag::is_skin_textures, checked);
      this->refreshTextureList();
   });
   QObject::connect(this->ui.flagModelSpaceNormals, &QCheckBox::toggled, this, [this](bool checked) {
      cobb::edit_bit(this->form->texture_flags, form_type::texture_set_flag::has_model_space_normals, checked);
      this->refreshTextureList();
   });
   //
   QObject::connect(this->ui.paths, &QTableWidget::currentCellChanged, this, [this](int currentRow, int currentColumn, int previousRow, int previousColumn) {
      if (currentRow >= 0 && currentRow <= 7) {
         if (auto* s = this->form->texture_by_index(currentRow)) {
            this->ui.editPath->setEnabled(true);
            this->ui.editPath->setPath(QString::fromStdString(*s));
            return;
         }
      }
      this->ui.editPath->setEnabled(false);
      this->ui.editPath->setPath(QString());
   });


   //
   this->ui.havokMaterialType->setAllowedFormType(dovah::form_type::material_type);
   this->ui.textureset->setAllowedFormType(dovah::form_type::texture_set);
   QObject::connect(this->ui.textureset, &FormPicker::formChanged, this, [this]() {
      //
      // TODO: Display a preview image of the selected textureset.
      //
   });
   //
   this->load();
}

namespace {
   void _set_row_text(QTableWidget* widget, int row, const QString& label, const std::string& value) {
      auto* head = widget->item(row, 0);
      auto* body = widget->item(row, 1);
      assert(head);
      assert(body);
      head->setText(label);
      body->setText(QString::fromStdString(value));
   }
   std::string _get_row_value(QTableWidget* widget, int row) {
      auto* body = widget->item(row, 1);
      assert(body);
      return body->text().toStdString();
   }
}
void FormDialogTextureSet::refreshTextureList() {
   bool skin = this->ui.flagSkinTexture->isChecked();
   bool msn  = this->ui.flagModelSpaceNormals->isChecked();
   //
   auto* widget = this->ui.paths;
   _set_row_text(widget, 0, tr("Diffuse",      "TextureSet map name"), this->form->texture_by_index<0>());
   _set_row_text(widget, 1, tr("Normal/Gloss", "TextureSet map name"), this->form->texture_by_index<1>());
   if (skin) {
      _set_row_text(widget, 2, tr("Subsurface Tint",  "TextureSet map name"), this->form->texture_by_index<2>());
      _set_row_text(widget, 3, tr("Detail Map",       "TextureSet map name"), this->form->texture_by_index<3>());
   } else {
      _set_row_text(widget, 2, tr("Environment Mask", "TextureSet map name"), this->form->texture_by_index<2>());
      _set_row_text(widget, 3, tr("Glow",             "TextureSet map name"), this->form->texture_by_index<3>());
   }
   _set_row_text(widget, 4, tr("Height",      "TextureSet map name"), this->form->texture_by_index<4>());
   _set_row_text(widget, 5, tr("Environment", "TextureSet map name"), this->form->texture_by_index<5>());
   _set_row_text(widget, 6, tr("Multilayer",  "TextureSet map name"), this->form->texture_by_index<6>());
   if (msn) {
      _set_row_text(widget, 7, tr("Specular",       "TextureSet map name"), this->form->texture_by_index<7>());
   } else {
      _set_row_text(widget, 7, tr("Backlight Mask", "TextureSet map name"), this->form->texture_by_index<7>());
   }
   //
   for (size_t i = 4; i < 7; ++i) {
      if constexpr (hide_inapplicable_paths) {
         widget->setRowHidden(i, skin);
      } else {
         auto  brush = skin ? QColor(192, 0, 0) : QBrush();
         auto* head  = widget->item(i, 0);
         auto* body  = widget->item(i, 1);
         head->setForeground(brush);
         body->setForeground(brush);
      }
   }
   if constexpr (hide_inapplicable_paths) {
      widget->setRowHidden(7, skin && !msn);
   } else {
      auto  brush = skin ? QColor(192, 0, 0) : QBrush();
      auto* head  = widget->item(7, 0);
      auto* body  = widget->item(7, 1);
      head->setForeground(brush);
      body->setForeground(brush);
   }
}

void FormDialogTextureSet::_load_impl() {
   auto& editor = DovahKitCore::get();
   //
   this->ui.editorID->setText(QString::fromStdString(this->stub->get_editor_id()));
   this->ui.editPath->setPath(QString());
   this->ui.flagModelSpaceNormals->setChecked(this->form->texture_flags & form_type::texture_set_flag::has_model_space_normals);
   this->ui.flagSkinTexture->setChecked(this->form->texture_flags & form_type::texture_set_flag::is_skin_textures);
   this->ui.flagSpecular->setChecked(!(this->form->texture_flags & form_type::texture_set_flag::no_specular_map));
   //
   if (this->form->decal_data) {
      auto& decal = *this->form->decal_data;
      this->ui.decalAlphaBlend->setChecked(decal.flags & decal_type::flag::alpha_blending);
      this->ui.decalAlphaTest->setChecked(decal.flags & decal_type::flag::alpha_testing);
      this->ui.decalFlagParallax->setChecked(decal.flags & decal_type::flag::parallax);
      this->ui.decalFlagSubtextures->setChecked(!(decal.flags & decal_type::flag::no_subtextures));
      this->ui.decalColor->setColor(QColor(decal.color.r, decal.color.g, decal.color.b));
      this->ui.decalShininess->setValue(decal.shininess);
      this->ui.decalDepth->setValue(decal.depth);
      this->ui.decalMinWidth->setValue(decal.width.min);
      this->ui.decalMaxWidth->setValue(decal.width.max);
      this->ui.decalMinHeight->setValue(decal.height.min);
      this->ui.decalMaxHeight->setValue(decal.height.max);
      this->ui.decalParallaxPasses->setValue(decal.parallax.passes);
      this->ui.decalScale->setValue(decal.parallax.scale);
   }
   //
   this->refreshTextureList();
}
void FormDialogTextureSet::_save_impl() {
   auto& editor = DovahKitCore::get();
   //
   this->stub->editorID = this->ui.editorID->text().toStdString();
   cobb::edit_bit(this->form->texture_flags, form_type::texture_set_flag::has_model_space_normals, this->ui.flagModelSpaceNormals->isChecked());
   cobb::edit_bit(this->form->texture_flags, form_type::texture_set_flag::is_skin_textures,        this->ui.flagSkinTexture->isChecked());
   cobb::edit_bit(this->form->texture_flags, form_type::texture_set_flag::no_specular_map,         !this->ui.flagSpecular->isChecked());
   //
   auto* widget = this->ui.paths;
   this->form->texture_by_index<0>() = _get_row_value(widget, 0);
   this->form->texture_by_index<1>() = _get_row_value(widget, 1);
   this->form->texture_by_index<2>() = _get_row_value(widget, 2);
   this->form->texture_by_index<3>() = _get_row_value(widget, 3);
   this->form->texture_by_index<4>() = _get_row_value(widget, 4);
   this->form->texture_by_index<5>() = _get_row_value(widget, 5);
   this->form->texture_by_index<6>() = _get_row_value(widget, 6);
   this->form->texture_by_index<7>() = _get_row_value(widget, 7);
   //
   if (this->ui.decalData->isChecked()) {
      auto* p = this->form->decal_data;
      if (!p) {
         p = this->form->decal_data = new decal_type;
      }
      //
      auto& decal = *p;
      {
         auto c = this->ui.decalColor->color();
         decal.color.r = c.red();
         decal.color.g = c.green();
         decal.color.b = c.blue();
      }
      cobb::edit_bit(decal.flags, decal_type::flag::alpha_blending, this->ui.decalAlphaBlend->isChecked());
      cobb::edit_bit(decal.flags, decal_type::flag::alpha_testing,  this->ui.decalAlphaTest->isChecked());
      cobb::edit_bit(decal.flags, decal_type::flag::parallax,       this->ui.decalFlagParallax->isChecked());
      cobb::edit_bit(decal.flags, decal_type::flag::no_subtextures, !this->ui.decalFlagSubtextures->isChecked());
      decal.width.min  = this->ui.decalMinWidth->value();
      decal.width.max  = this->ui.decalMaxWidth->value();
      decal.height.min = this->ui.decalMinHeight->value();
      decal.height.max = this->ui.decalMaxHeight->value();
      decal.depth      = this->ui.decalDepth->value();
      decal.shininess  = this->ui.decalShininess->value();
      decal.parallax.passes = this->ui.decalParallaxPasses->value();
      decal.parallax.scale  = this->ui.decalScale->value();
   } else {
      if (auto*& p = this->form->decal_data) {
         delete p;
         p = nullptr;
      }
   }
}