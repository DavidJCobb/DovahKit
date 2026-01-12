#pragma once
#include <concepts>
#include <cstdint>
#include <optional>
#include <vector>
#include <QAbstractItemModel>
#include <QByteArray>
#include <QDataStream>
#include "./drag_drop_node_id_map.h"

namespace ui::model_utils::drag_drop_nodes_by_id {
   namespace impl {
      template<typename Functor, typename Node>
      concept qmi_to_mutable_node_pointer_functor = requires(Functor&& f, const QModelIndex& qmi) {
         { f(qmi) } -> std::same_as<Node*>;
      };
      template<typename Functor, typename Node>
      concept qmi_to_const_node_pointer_functor = requires(Functor&& f, const QModelIndex& qmi) {
         { f(qmi) } -> std::same_as<const Node*>;
      };

      template<typename Functor, typename Node>
      concept qmi_to_node_pointer_functor = qmi_to_mutable_node_pointer_functor<Functor, Node> || qmi_to_const_node_pointer_functor<Functor, Node>;
   }

   template<typename Node, impl::qmi_to_node_pointer_functor<Node> Functor>
   QByteArray build_data(
      const QModelIndexList&       indices,
      const QAbstractItemModel&    model,
      drag_drop_node_id_map<Node>& node_id_map,
      Functor&&                    qmi_to_node_functor
   ) {
      QByteArray  data;
      QDataStream stream(&data, QIODevice::WriteOnly);
      //
      // Stream begins with the model pointer. The pointer is used only for equality 
      // checks on drop (i.e. no moving/copying data across models) and will never be 
      // dereferenced.
      //
      stream << (intptr_t)&model;
      //
      for (const QModelIndex& qmi : indices) {
         const Node* node = qmi_to_node_functor(qmi);
         if (!node)
            continue;
         stream << node_id_map.track(*const_cast<Node*>(node));
      }

      return data;
   }

   template<typename Node>
   std::optional<std::vector<Node*>> extract_nodes_from_data(
      const QByteArray&            data,
      const QAbstractItemModel&    model,
      drag_drop_node_id_map<Node>& node_id_map
   ) {
      QDataStream stream(data);
      {  // Verify that this is an internal move.
         std::intptr_t model_pointer;
         stream >> model_pointer;
         if ((QAbstractItemModel*)model_pointer != &model)
            return {};
      }
      std::vector<Node*> dragged_nodes;
      while (!stream.atEnd()) {
         Node* node = nullptr;
         {
            typename drag_drop_node_id_map<Node>::uid_type id;
            stream >> id;
            node = node_id_map.get_by_id(id);
         }
         if (!node)
            continue;
         dragged_nodes.push_back(node);
      }
      return dragged_nodes;
   }
}
