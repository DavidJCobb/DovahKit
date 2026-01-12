#include "./objects.h"
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTreeView>
#include <QSpinBox>
#include "helpers/bound_mem_fn.h"
#include "widgets/DKColorPickerButton.h"
#include "ui/utils/set_range.h"
#include "../RegionObjectsModel.h"
#include "../RegionsDialog.h"

// Given: DO(control, object_data_field, ...)
#define FOR_EACH_NUMERIC_PARAM(DO) \
   DO(edit.angle_variance.ranges.x,   angle_variance.x) \
   DO(edit.angle_variance.ranges.y,   angle_variance.y) \
   DO(edit.angle_variance.ranges.z,   angle_variance.z) \
   DO(edit.clustering,                clustering) \
   DO(edit.density,                   density) \
   DO(edit.height.min,                height.min) \
   DO(edit.height.max,                height.max) \
   DO(edit.paint_vertices.percentage, paint_vertices.radius_percent) \
   DO(edit.radius,                    radius) \
   DO(edit.radius_wrt_parent,         radius_wrt_parent) \
   DO(edit.sink.base,                 sink.base) \
   DO(edit.sink.variance,             sink.variance) \
   DO(edit.size_variance.range,       size_variance) \
   DO(edit.slope.min,                 slope.min) \
   DO(edit.slope.max,                 slope.max)

// Given: DO(control, flag_name, ...)
#define FOR_EACH_FLAG_FIELD(DO) \
   DO(edit.angle_variance.invertible.x, angle_x_range_signed) \
   DO(edit.angle_variance.invertible.y, angle_y_range_signed) \
   DO(edit.angle_variance.invertible.z, angle_z_range_signed) \
   DO(edit.conform_to_slope,            conform_to_slope) \
   DO(edit.is_huge_rock,                huge_rock) \
   DO(edit.is_tree,                     tree) \
   DO(edit.paint_vertices.enabled,      paint_vertices) \
   DO(edit.size_variance.invertible,    size_variance_signed)

namespace ui::region::fragments {
   objects::objects(RegionsDialog& o) : owner(o) {
      this->model = new model_type(&o);
   }
   void objects::set_controls(controls&& src) {
      this->ui = std::move(src);
      this->ui.edit.container->setEnabled(false);
      this->ui.edit.base_editor_id->setReadOnly(true);

      {
         auto* view = this->ui.view;
         view->setModel(this->model);
         view->setDragDropMode(QAbstractItemView::DragDropMode::InternalMove);
         view->setDragDropOverwriteMode(false);
         view->setDragEnabled(true);
         view->setAcceptDrops(true);
         view->setDropIndicatorShown(true);

         QObject::connect(view->selectionModel(), &QItemSelectionModel::selectionChanged, &this->owner, [this](const QItemSelection& sel) {
            if (sel.empty()) {
               this->on_no_object_selected();
            } else {
               this->on_object_selected(sel[0].topLeft());
            }
         });

         //
         // TODO: "Move Up" button
         // 
         // TODO: "Move Down" button
         // 
         // TODO: Context menu
         // 
         // TODO: "Del" keybind
         //
      }

      QObject::connect(this->ui.header.enable,  &QCheckBox::toggled, &this->owner, cobb__bound_this_fn(on_header_edited));
      QObject::connect(this->ui.header.override, &QCheckBox::toggled, &this->owner, cobb__bound_this_fn(on_header_edited));
      QObject::connect(this->ui.header.priority, qOverload<int>(&QSpinBox::valueChanged), &this->owner, cobb__bound_this_fn(on_header_edited));

      for (auto* widget : std::array{
         this->ui.edit.angle_variance.ranges.x,
         this->ui.edit.angle_variance.ranges.y,
         this->ui.edit.angle_variance.ranges.z,
         this->ui.edit.density,
         this->ui.edit.height.min,
         this->ui.edit.height.max,
         this->ui.edit.sink.base,
         this->ui.edit.sink.variance,
         this->ui.edit.size_variance.range,
      }) {
         QObject::connect(widget, qOverload<double>(&QDoubleSpinBox::valueChanged), &this->owner, cobb__bound_this_fn(on_object_edited));
      }
      for (auto* widget : std::array{
         this->ui.edit.clustering,
         this->ui.edit.paint_vertices.percentage,
         this->ui.edit.radius,
         this->ui.edit.radius_wrt_parent,
         this->ui.edit.slope.min,
         this->ui.edit.slope.max,
      }) {
         QObject::connect(widget, qOverload<int>(&QSpinBox::valueChanged), &this->owner, cobb__bound_this_fn(on_object_edited));
      }
      for (auto* widget : std::array{
         #define DO(_control, ...) this->ui._control,
         FOR_EACH_FLAG_FIELD(DO)
         #undef DO
      }) {
         QObject::connect(widget, &QCheckBox::toggled, &this->owner, cobb__bound_this_fn(on_object_edited));
      }
      QObject::connect(this->ui.edit.paint_vertices.color, &DKColorPickerButton::colorChanged, &this->owner, cobb__bound_this_fn(on_object_edited));
   }
   void objects::reload() {
      auto& data     = this->owner.region_data({});
      auto& opt_coll = data.generable_content.objects;
      this->ui.header.enable->setEnabled(data.stub != nullptr);
      if (!data.stub || !opt_coll.has_value()) {
         this->ui.header.enable->setChecked(false);
         this->ui.header.override->setChecked(false);
         this->ui.header.override->setEnabled(false);
         this->ui.header.priority->setValue(0);
         this->ui.header.priority->setEnabled(false);
         this->model->clear();
         this->ui.view->setEnabled(false);
         return;
      }
      auto& src = opt_coll.value();
      this->ui.header.enable->setChecked(true);
      this->ui.header.override->setChecked(src.override);
      this->ui.header.override->setEnabled(true);
      this->ui.header.priority->setValue(src.priority);
      this->ui.header.priority->setEnabled(true);
      this->ui.view->setEnabled(true);
      this->model->importData(data);
   }
   void objects::commit() {
      auto& data     = this->owner.region_data({});
      auto& opt_coll = data.generable_content.objects;
      if (this->ui.header.enable->isChecked()) {
         opt_coll.emplace();
         auto& dst = opt_coll.value();
         dst.override = this->ui.header.override->isChecked();
         dst.priority = this->ui.header.priority->value();
         this->model->exportData(data);
      } else {
         opt_coll.reset();
      }
   }

   void objects::on_object_selected(const QModelIndex& qmi) {
      auto data = qmi.data(model_type::ObjectDataRole).value<model_type::object_data>();
      if (!data.base_form) {
         this->on_no_object_selected();
         return;
      }

      this->ui.edit.container->setEnabled(true);
      {
         auto text = QString::fromStdString(data.base_form->editorID);
         this->ui.edit.base_editor_id->setText(text);
      }

      const auto blockers = std::array{
         #define DO(control, ...) QSignalBlocker(this->ui.control),
         FOR_EACH_NUMERIC_PARAM(DO)
         FOR_EACH_FLAG_FIELD(DO)
         #undef DO
         QSignalBlocker(this->ui.edit.paint_vertices.color),
      };
      
      using flag = model_type::object_data::object_params::flag;

      #define DO(_control, _param, ...) this->ui._control->setValue(data.params._param);
      FOR_EACH_NUMERIC_PARAM(DO);
      #undef DO
      #define DO(_control, _flag, ...) this->ui._control->setChecked(data.params.flags & flag::_flag);
      FOR_EACH_FLAG_FIELD(DO)
      #undef DO
      this->ui.edit.paint_vertices.color->setColor(QColor(data.params.paint_vertices.color.r, data.params.paint_vertices.color.g, data.params.paint_vertices.color.b));
   }
   void objects::on_no_object_selected() {
      this->ui.edit.container->setEnabled(false);
   }
   void objects::on_header_edited() {
      bool enabled = this->ui.header.enable->isChecked();
      this->ui.header.override->setEnabled(enabled);
      this->ui.header.priority->setEnabled(enabled);
      this->ui.edit.container->setEnabled(enabled);
      this->ui.view->setEnabled(enabled);
      this->owner.on_region_modified({});
   }
   void objects::on_object_edited() {
      auto sel = this->ui.view->selectionModel()->selection();
      if (sel.empty())
         return;
      auto qmi  = sel[0].topLeft();
      auto data = qmi.data(model_type::ObjectDataRole).value<model_type::object_data>();

      using flag = model_type::object_data::object_params::flag;

      #define DO(_control, _param, ...) data.params._param = this->ui._control->value();
      FOR_EACH_NUMERIC_PARAM(DO);
      #undef DO
      #define DO(_control, _flag, ...) cobb::edit_bit(data.params.flags, flag::_flag, this->ui._control->isChecked());
      FOR_EACH_FLAG_FIELD(DO)
      #undef DO
      {
         auto  color = this->ui.edit.paint_vertices.color->color();
         auto& param = data.params.paint_vertices.color;
         param.r = color.red();
         param.g = color.green();
         param.b = color.blue();
      }

      this->model->setData(qmi, QVariant::fromValue(data), model_type::ObjectDataRole);
      this->owner.on_region_modified({});
   }
}