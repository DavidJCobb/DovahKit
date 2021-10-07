#include "DovahscriptCanvasExporter.h"
#include "../../../ui/generic/CanvasWidget.h"

void DovahscriptCanvasExporter::process(CanvasWidget* canvas) {
   this->index_file.clear();
   this->layers.clear();
   this->groups.clear();
   if (!canvas)
      return;
   this->gather_layers(*canvas);
   //
   {
      this->index_file += QObject::tr("== Groups ==\n\n", "script canvas layer export");
      auto size = groups.size();
      for (int i = 0; i < size; ++i)
         this->index_file += entity_as_text(i + 1, *groups[i]);
      if (size == 0)
         this->index_file += QObject::tr("(None.)", "script canvas layer export");
   }
   {
      this->index_file += QObject::tr("== Layers ==\n\n", "script canvas layer export");
      auto size = layers.size();
      for (int i = 0; i < size; ++i)
         this->index_file += entity_as_text(i + 1, *layers[i]);
      if (size == 0)
         this->index_file += QObject::tr("(None.)", "script canvas layer export");
   }
   this->index_file += QObject::tr("== Structure ==\nNote: layers later in the list are drawn overtop layers earlier in the list\n\n", "script canvas layer export");
   this->print_layers(*canvas);
}

void DovahscriptCanvasExporter::gather_layers(const CanvasWidget& canvas) {
   for (auto* entity : canvas.layers()) {
      if (entity->isLayerGroup()) {
         auto* group = (CanvasWidgetLayerGroup*)entity;
         this->groups.push_back(group);
         this->gather_layers(*group);
      } else {
         auto* layer = (CanvasWidgetLayer*)entity;
         this->layers.push_back(layer);
      }
   }
}
void DovahscriptCanvasExporter::gather_layers(const CanvasWidgetLayerGroup& group) {
   for (auto* entity : group.childLayers()) {
      if (entity->isLayerGroup()) {
         auto* nested = (CanvasWidgetLayerGroup*)entity;
         this->groups.push_back(nested);
         this->gather_layers(*nested);
      } else {
         auto* layer = (CanvasWidgetLayer*)entity;
         this->layers.push_back(layer);
      }
   }
}

void DovahscriptCanvasExporter::print_layers(const CanvasWidget& canvas) {
   for (auto* entity : canvas.layers()) {
      auto name = entity->objectName();
      if (!name.isEmpty())
         name = QString("(%1)").arg(name);
      if (entity->isLayerGroup()) {
         auto* group = (CanvasWidgetLayerGroup*) entity;
         this->index_file += QObject::tr(" - Group #%1 %2\n").arg(groups.indexOf(group) + 1).arg(name);
         this->print_layers(*group, "   ");
      } else {
         auto* layer = (CanvasWidgetLayer*) entity;
         this->index_file += QObject::tr(" - Layer #%1 %2\n").arg(layers.indexOf(layer) + 1).arg(name);
      }
   }
}
void DovahscriptCanvasExporter::print_layers(const CanvasWidgetLayerGroup& group, const QString& indent) {
   for (auto* entity : group.childLayers()) {
      int  position = -1;
      bool is_group = entity->isLayerGroup();
      CanvasWidgetLayerGroup* nested = nullptr;
      if (is_group) {
         nested = (CanvasWidgetLayerGroup*)entity;
         position = groups.indexOf(nested);
      } else {
         auto* layer = (CanvasWidgetLayer*)entity;
         position = layers.indexOf(layer);
      }
      assert(position >= 0);
      auto name = entity->objectName();
      if (!name.isEmpty())
         name = QString("(%1)").arg(name);
      //
      this->index_file += QObject::tr("%1 - %2 #%3 %4\n").arg(indent).arg(is_group ? "Group" : "Layer").arg(position + 1).arg(name);
      if (nested)
         this->print_layers(*nested, indent + "   ");
   }
}

/*static*/ QString DovahscriptCanvasExporter::entity_as_text(int index, const CanvasWidgetEntity& entity) {
   QString out;
   if (entity.isLayerGroup())
      out = QObject::tr("Group %1 %2", "script canvas layer export");
   else
      out = QObject::tr("Layer %1 %2", "script canvas layer export");
   out = out.arg(index);
   {
      auto name = entity.objectName();
      if (name.isEmpty())
         out = out.arg(name);
      else
         out = out.arg(QString("(%1)").arg(name));
   }
   out += '\n';
   out += QObject::tr(" - Visible:    %1\n").arg(entity.visible() ? "Yes" : "No");
   out += QObject::tr(" - Position:   (%1, %2)\n").arg(entity.position().x()).arg(entity.position().y());
   out += QObject::tr(" - Opacity:    %1\n").arg(entity.opacity());
   out += QObject::tr(" - Blend Mode: ");
   switch (entity.compositionMode()) {
      case CanvasWidgetEntity::CompositionMode::CompositionMode_SourceOver:
         out += QObject::tr("Normal");
         break;
      case CanvasWidgetEntity::CompositionMode::CompositionMode_ColorBurn:
         out += QObject::tr("Burn");
         break;
      case CanvasWidgetEntity::CompositionMode::CompositionMode_ColorDodge:
         out += QObject::tr("Dodge");
         break;
      case CanvasWidgetEntity::CompositionMode::CompositionMode_Darken:
         out += QObject::tr("Darken Only");
         break;
      case CanvasWidgetEntity::CompositionMode::CompositionMode_Difference:
         out += QObject::tr("Difference");
         break;
      case CanvasWidgetEntity::CompositionMode::CompositionMode_HardLight:
         out += QObject::tr("Hard Light");
         break;
      case CanvasWidgetEntity::CompositionMode::CompositionMode_Lighten:
         out += QObject::tr("Lighen Only");
         break;
      case CanvasWidgetEntity::CompositionMode::CompositionMode_Multiply:
         out += QObject::tr("Multiply");
         break;
      case CanvasWidgetEntity::CompositionMode::CompositionMode_Overlay:
         out += QObject::tr("Overlay");
         break;
      case CanvasWidgetEntity::CompositionMode::CompositionMode_Plus:
         out += QObject::tr("Add");
         break;
      case CanvasWidgetEntity::CompositionMode::CompositionMode_Screen:
         out += QObject::tr("Screen");
         break;
      case CanvasWidgetEntity::CompositionMode::CompositionMode_SoftLight:
         out += QObject::tr("Soft Light");
         break;
      default:
         out += QObject::tr("Unknown (%1)").arg((int)entity.compositionMode());
         break;
   }
   out += "\n\n";
   return out;
}