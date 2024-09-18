#pragma once
#include <vector>
#include "ui/models/DKGenericListModel.h"

namespace dovah {
   namespace loaded_forms {
      class Form;
   }
   class form_reference_t;
   class form_stub;
}

struct TopicInfoLinkedTopicsModelNode {
   dovah::form_stub* stub = nullptr; // topic
   bool locked = false;
   struct {
      QString topic_editor_id;
      QString branch_editor_id;
   } cached;

   void recache();
};

class TopicInfoLinkedTopicsModel : public DKGenericListModel<TopicInfoLinkedTopicsModel, TopicInfoLinkedTopicsModelNode> {
   public:
      struct Column {
         Column() = delete;
         enum enumeration : size_t {
            Branch,
            Topic,

            __COUNT
         };
      };
      static constexpr const size_t column_count = Column::__COUNT; // override

   public:
      TopicInfoLinkedTopicsModel(QObject* parent);

      using DKGenericListModel::clear;

      #pragma region Overrides
         QVariant data_of(const node_type& node, Qt::ItemDataRole role, size_t column) const;
         Qt::ItemFlags flags_of(const node_type&, size_t column) const;

         virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
      #pragma endregion

      void addTopic(dovah::form_stub*);
      void reorderTopic(size_t row, int by);
      void removeTopic(dovah::form_stub*);
      void removeTopic(size_t);

      inline const bool containsTopic(const dovah::form_stub* stub) const noexcept {
         for (auto* node : this->_nodes)
            if (node->stub == stub)
               return true;
         return false;
      }

      void importFrom(
         const std::vector<dovah::form_reference_t>& locked,
         const std::vector<dovah::form_reference_t>& normal
      );
      void exportTo(std::vector<dovah::form_reference_t>& normal, dovah::loaded_forms::Form& containing_form_of_list);
};