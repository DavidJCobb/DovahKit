#pragma once
#include <vector>
#include <QAbstractItemModel>
#include <QIdentityProxyModel>
#include <QSortFilterProxyModel>
#include "dovah/data/dialogue/category.h"
#include "dovah/form_stub.h"

class QuestAllDialogueModel : public QAbstractItemModel {
   Q_OBJECT;
   public:
      QuestAllDialogueModel(QObject* parent = nullptr);
      ~QuestAllDialogueModel();

   public:
      struct BranchColumn {
         BranchColumn() = delete;
         enum {
            EditorID,
            FormID,
            Flags, // B for blocking, T for top-level, E for exclusive; multiple allowed e.g. BE

            __COUNT
         };
      };
      static constexpr const size_t BranchColumnCount = BranchColumn::__COUNT;

      struct TopicColumn {
         TopicColumn() = delete;
         enum {
            EditorID,
            IsBranchStartingTopic,
            FormID,
            Priority,
            DisplayText,

            __COUNT
         };
      };
      static constexpr const size_t TopicColumnCount = TopicColumn::__COUNT;

      struct InfoColumn {
         InfoColumn() = delete;
         enum {
            InfoText, // if SharedInfo, prefixed with "<<Shared>> "
            EditorID, // default collapsed
            FormID,   // default collapsed
            Flags,
            ResponseCount,
            Speaker,
            Target,
            IsVoiceType,
            InFaction,
            Conditions,
            HasResultScript,

            __COUNT
         };
      };
      static constexpr const size_t InfoColumnCount = InfoColumn::__COUNT;

   protected:
      struct Info {
         dovah::form_stub* stub = nullptr;
         bool deleted = false;
         struct {
            QString  editor_id;
            size_t   response_count = 0;
            QString  responses;
            QString  speaker;
            QString  target;
            QString  voicetype;
            QString  faction;
            QString  conditions;
            bool     links_to_any_topics = false;
            bool     has_end_fragment    = false;
            bool     has_own_prompt      = false;
            bool     uses_shared_info    = false;
            uint16_t hours_until_reset   = 0;
            uint32_t flags = 0;
         } cached;

         void recache_from_stub(
            dovah::form_stub& owning_quest
         );
      };

      struct Topic {
         ~Topic();

         dovah::form_stub*  stub = nullptr;
         std::vector<Info*> infos;
         struct {
            QString  editor_id;
            QString  display_text;
            uint8_t  priority = 0;
            uint32_t subtype  = 0; // signature

            dovah::dialogue::category category = dovah::dialogue::category::topic;
         } cached;

         void recache_from_stub();
      };

      struct Branch {
         enum class Type {
            Normal,
            Blocking,
            TopLevel,
         };

         ~Branch();

         dovah::form_stub*   stub = nullptr;
         std::vector<Topic*> topics;
         struct {
            QString editor_id;
            Type    type      = Type::Normal;
            bool    exclusive = false;
         } cached;
         Topic* starting_topic = nullptr;

         void recache_from_stub();
      };

      struct {
         std::vector<Branch*> branches;
         std::vector<Topic*>  branchless_topics; // generally anything that isn't Player Dialogue
      } _data;
      dovah::form_stub* _quest = nullptr;

      void _clear(bool emit_signals);

      QModelIndex _qmi_for_form(dovah::form_stub&, size_t row = 0, size_t col = 0) const;
      dovah::form_stub* _form_from_qmi(const QModelIndex&) const;

      Branch* _branch_for_stub(const dovah::form_stub&) const;
      Topic* _topic_for_stub(const dovah::form_stub&) const;
      Info* _info_for_stub(const dovah::form_stub&) const;

   public:
      #pragma region QAbstractItemModel overrides
         #pragma region Hierarchy
            virtual QModelIndex index(int row, int column, const QModelIndex& parent) const override;
            virtual QModelIndex parent(const QModelIndex& index) const override;
            virtual QModelIndex sibling(int row, int column, const QModelIndex& index) const override;
            virtual int         rowCount(const QModelIndex& parent) const override;
            virtual int         columnCount(const QModelIndex& parent) const override;
         #pragma endregion
         #pragma region Node data
            virtual QVariant      data(const QModelIndex& index, int role) const override;
            virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
         #pragma endregion
         virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
      #pragma endregion

      void setQuest(dovah::form_stub*);

      QModelIndex branchlessTopicRoot() const;

      QModelIndex index(const dovah::form_stub&) const;

   protected:
      void _on_branch_edited(dovah::form_stub&);
      void _on_topic_edited(dovah::form_stub&);
      void _on_info_edited(dovah::form_stub&);

      void _on_branch_created(dovah::form_stub&);
      void _on_topic_created(dovah::form_stub&);
      void _on_info_created(dovah::form_stub&);

      void _on_branch_removed(dovah::form_stub&);
      void _on_topic_removed(dovah::form_stub&);

      // CK behavior:
      //  - Infos that are flagged as deleted still display in the listing.
      //  - Topics that are flagged as deleted are hidden entirely.
      //  - Branches that are flagged as deleted are probably also hidden entirely.
      void _on_form_deleted(dovah::form_stub&, bool just_being_flagged);
};

class QuestBranchesModel : public QSortFilterProxyModel {
   Q_OBJECT;
   public:
      using Column = QuestAllDialogueModel::BranchColumn;
      static constexpr const size_t ColumnCount = Column::__COUNT;

   public:
      //
      // TODO
      // 
      //  - Branches are sorted alphabetically by editor ID.
      //  - You can filter to just top-level branches.
      //

      virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

      virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;
      virtual bool lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const override;
};

class QuestBranchTopicsModel : public QSortFilterProxyModel {
   Q_OBJECT;
   public:
      using Column = QuestAllDialogueModel::TopicColumn;
      static constexpr const size_t ColumnCount = Column::__COUNT;

   public:
      //
      // TODO
      // 
      //  - Topics are sorted alphabetically by editor ID.
      // 
      //     - Does the CK also sort topics by priority? If so, which is the 
      //       main sort and which is the sub-sort?
      //

      virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

      virtual bool lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const override;
};

class QuestBranchlessTopicsModel : public QSortFilterProxyModel {
   Q_OBJECT;
   public:
      using Column = QuestAllDialogueModel::TopicColumn;
      static constexpr const size_t ColumnCount = Column::__COUNT;

   public:
      //
      // TODO: filter by list of allowed topic subtypes (or, alternatively, by 
      // category, with the model doing the work of figuring out what subtypes 
      // are eligible).
      // 
      // TODO: Same sorting as QuestBranchTopicsModel.
      //

      virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

      virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;
      virtual bool lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const override;
};

class QuestTopicInfoModel : public QIdentityProxyModel {
   Q_OBJECT;
   public:
      using Column = QuestAllDialogueModel::InfoColumn;
      static constexpr const size_t ColumnCount = Column::__COUNT;

   public:
      //
      // TODO
      // 
      //  - Infos are not sorted. We just need to change the column headers.
      //

      virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
};