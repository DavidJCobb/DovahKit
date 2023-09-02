#include "./EditGizmoColorSchemeModel.h"
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include "editor/subsystems/worldedit/gizmo_colors/edit_gizmo_color_scheme_manager.h"

void EditGizmoColorSchemeModelNode::update_icon() {
   QImage image(16, 16, QImage::Format::Format_ARGB32);

   QPainter painter(&image);
   painter.setPen(Qt::PenStyle::NoPen);

   QPointF points[4];

   painter.setBrush(QBrush(QColor::fromRgb(this->data.axis_x.r, this->data.axis_x.g, this->data.axis_x.b)));
   points[0] = { 0,  0 };
   points[1] = { 8,  0 };
   points[2] = { 8,  8 };
   points[3] = { 0, 16 };
   painter.drawPolygon(points, 4);

   painter.setBrush(QBrush(QColor::fromRgb(this->data.axis_y.r, this->data.axis_y.g, this->data.axis_y.b)));
   points[0] = { 16,  0 };
   points[1] = {  8,  0 };
   points[2] = {  8,  8 };
   points[3] = { 16, 16 };
   painter.drawPolygon(points, 4);

   painter.setBrush(QBrush(QColor::fromRgb(this->data.axis_z.r, this->data.axis_z.g, this->data.axis_z.b)));
   points[0] = {  0, 16 };
   points[1] = {  8,  8 };
   points[2] = { 16, 16 };
   painter.drawPolygon(points, 3);

   painter.setPen(QColor::fromRgb(0, 0, 0));
   painter.setBrush(QBrush(QColor::fromRgb(this->data.highlight.r, this->data.highlight.g, this->data.highlight.b)));
   painter.drawEllipse(QPoint{ 8, 8 }, 3, 3);

   painter.setBrush(Qt::BrushStyle::NoBrush);
   painter.drawRect(0, 0, 15, 15);

   this->icon = QIcon(QPixmap::fromImage(image));
}

QVariant EditGizmoColorSchemeModel::data_of(const node_type& node, Qt::ItemDataRole role, size_t column) const {
   switch (role) {
      case IsHardcodedRole:
         return node.is_hardcoded;
      case Qt::DisplayRole:
      case Qt::ToolTipRole:
         return node.name;
      case Qt::DecorationRole:
         return node.icon;
   }
   return {};
}
Qt::ItemFlags EditGizmoColorSchemeModel::flags_of(const node_type& node, size_t column) const {
   Qt::ItemFlags flags = {};
   flags |= Qt::ItemFlag::ItemIsEnabled;
   flags |= Qt::ItemFlag::ItemIsSelectable;
   return flags;
}

void EditGizmoColorSchemeModel::reset_from_options() {
   this->performReset([this]() {
      auto& manager = dovahkit::subsystems::worldedit::gizmo_color_scheme_manager::get();
      {
         auto list = manager.all_hardcoded_color_schemes();
         for (const auto& item : list) {
            auto* node = new node_type{};
            this->_nodes.push_back(node);
            node->name = QString::fromUtf8(item.name.c_str(), item.name.size());
            node->data = item;
            node->is_hardcoded = true;
            node->update_icon();
         }
      }
      {
         const auto& list = manager.all_user_color_schemes();
         for (const auto& item : list) {
            auto* node = new node_type{};
            this->_nodes.push_back(node);
            node->name = QString::fromUtf8(item.name.c_str(), item.name.size());
            node->data = item;
            node->is_hardcoded = false;
            node->update_icon();
         }
      }
   });
}
void EditGizmoColorSchemeModel::force_replace_options(const QModelIndex& selected) {
   auto& manager = dovahkit::subsystems::worldedit::gizmo_color_scheme_manager::get();

   std::vector<data_type> replace_with;
   for (const auto* item : this->_nodes) {
      if (item->is_hardcoded)
         continue;
      replace_with.push_back(item->data);
   }
   manager.replace_all_user_schemes(replace_with);

   if (selected.isValid()) {
      auto* node = this->node(selected);
      if (node) {
         manager.set_current_color_scheme_id({
            .name         = node->data.name,
            .is_hardcoded = node->is_hardcoded,
         });
      }
   }
}

std::optional<EditGizmoColorSchemeModel::data_type> EditGizmoColorSchemeModel::dataFor(const QModelIndex& qmi) const {
   if (auto* node = this->node(qmi))
      return node->data;
   return {};
}
void EditGizmoColorSchemeModel::replaceDataFor(const QModelIndex& qmi, const data_type& data) {
   if (auto* node = this->node(qmi)) {
      node->data = data;
      this->emitNodeChanged(*node);
   }
}

QModelIndex EditGizmoColorSchemeModel::insert(const data_type& data) {
   this->beginInsertRows({}, this->_nodes.size(), this->_nodes.size());

   auto name = QString::fromUtf8(data.name.c_str(), data.name.size());
   {
      auto   candidate = name;
      size_t alternate = 2;
      if (name.endsWith(')')) {
         auto i = name.lastIndexOf('(');
         if (i > 0) {
            bool ok;
            alternate = name.mid(i, name.size() - i - 2).toInt(&ok);
            if (ok)
               candidate = name.left(i);
            else
               alternate = 2;
         }
      }

      while (true) {
         bool found = false;
         for (const auto* prior : this->_nodes) {
            if (prior->name == name) {
               found = true;
               break;
            }
         }
         if (!found)
            break;
         name = candidate + " (" + QString::number(alternate++) + ")";
      }
   }

   auto* node = new node_type;
   this->_nodes.push_back(node);
   node->data = data;
   node->name = name;
   node->is_hardcoded = false;
   node->update_icon();

   this->endInsertRows();

   return index(node);
}