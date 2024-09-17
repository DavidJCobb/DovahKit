#include "./TopicInfoResponseTableviewModel.h"
#include "dovah/form_stub.h"
#include "editor/core.h"

TopicInfoResponseTableviewModel::TopicInfoResponseTableviewModel(QObject* parent) : DKGenericListModel(parent) {
}

QVariant TopicInfoResponseTableviewModel::data_of(const node_type& node, Qt::ItemDataRole role, size_t column) const {
   switch (role) {
      case Qt::TextAlignmentRole:
         switch (column) {
            case Column::Emotion:
            case Column::Edited:
               return (int)(Qt::AlignHCenter | Qt::AlignVCenter);
         }
         return {};

      case Qt::DisplayRole:
      case Qt::ToolTipRole:
         switch (column) {
            case Column::Text:
               return node.response_text;
            case Column::Emotion:
               {
                  auto text = tr("%1 (%2)");
                  switch (node.emotion.type) {
                     using enum dovah::dialogue::emotion;
                     case neutral:
                        text = text.arg(tr("Neutral"));
                        break;
                     case anger:
                        text = text.arg(tr("Anger"));
                        break;
                     case disgust:
                        text = text.arg(tr("Disgust"));
                        break;
                     case fear:
                        text = text.arg(tr("Fear"));
                        break;
                     case sad:
                        text = text.arg(tr("Sad"));
                        break;
                     case happy:
                        text = text.arg(tr("Happy"));
                        break;
                     case surprise:
                        text = text.arg(tr("Surprise"));
                        break;
                     case puzzled:
                        text = text.arg(tr("Puzzled"));
                        break;
                     default:
                        text = text.arg(tr("?"));
                        break;
                  }
                  text = text.arg(node.emotion.value);
                  return text;
               }
               break;
            case Column::Edited:
               return node.edited ? tr("Y") : tr("N");
         }
         return {};
   }
   return {};
}
Qt::ItemFlags TopicInfoResponseTableviewModel::flags_of(const node_type&, size_t column) const {
   auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
   return flags;
}

/*virtual*/ QVariant TopicInfoResponseTableviewModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
   if (orientation != Qt::Orientation::Horizontal)
      return {};
   if (role == Qt::TextAlignmentRole) {
      switch (section) {
         case Column::Emotion:
         case Column::Edited:
            return (int)(Qt::AlignHCenter | Qt::AlignVCenter);
      }
      return {};
   }
   if (role != Qt::DisplayRole)
      return {};
   switch (section) {
      using enum Column::enumeration;
      case Text:
         return tr("Response Text");
      case Emotion:
         return tr("Emotion");
      case Edited:
         return tr("Edited");
   }
   return {};
}

std::optional<size_t> TopicInfoResponseTableviewModel::create() {
   auto i = this->_nodes.size();
   this->beginInsertRows({}, i, i);
   this->_nodes.push_back(new node_type{});
   this->endInsertRows();
   return i;
}
void TopicInfoResponseTableviewModel::overwrite(size_t row, const node_type& src) {
   if (row < 0 || row >= this->_nodes.size())
      return;
   auto* node = this->_nodes[row];
   *node = src;
   
   auto tl = this->index(row, 0, {});
   auto br = this->index(row, column_count, {});
   emit dataChanged(tl, br);
}

void TopicInfoResponseTableviewModel::overwriteAllItems(const std::vector<node_type>& src) {
   this->performReset([this, &src]() {
      size_t size = src.size();
      this->_nodes.resize(size);
      for (size_t i = 0; i < size; ++i) {
         auto* node = this->_nodes[i] = new node_type{ src[i] };
         *node = src[i];
      }
   });
}