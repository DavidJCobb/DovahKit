#pragma once
#include <QMenu>
#include "./_base.h"
#include "dovah/forms/TopicInfo.h"
#include "ui_topic_info.h" // generated

namespace dovah::loaded_forms::components::papyrus {
   class topic_info_fragment_data;
}
class TopicInfoLinkedTopicsModel;
class TopicInfoResponseTableviewModel;
class TopicInfoSharedInfoFormFilter;
class TopicInfoWalkAwayTopicFormFilter;

class FormDialogTopicInfo :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::TopicInfo, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   protected:
      using papyrus_fragment_data_type = dovah::loaded_forms::components::papyrus::topic_info_fragment_data;

   public:
      FormDialogTopicInfo(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogTopicInfo ui;
      struct {
         struct {
            QMenu menu;
            struct {
               QAction* create    = nullptr;
               QAction* move_up   = nullptr;
               QAction* move_down = nullptr;
               QAction* remove    = nullptr;
            } actions;
         } linked_topics;
         struct {
            QMenu menu;
            struct {
               QAction* create    = nullptr;
               QAction* edit      = nullptr;
               QAction* move_up   = nullptr;
               QAction* move_down = nullptr;
               QAction* remove    = nullptr;
            } actions;
         } responses;
      } _context_menus;
      struct {
         TopicInfoSharedInfoFormFilter*    shared_info     = nullptr;
         TopicInfoWalkAwayTopicFormFilter* walk_away_topic = nullptr;
      } _filters;
      struct {
         TopicInfoLinkedTopicsModel*      linked_topics = nullptr;
         TopicInfoResponseTableviewModel* responses     = nullptr;
      } _models;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;

      void _update_topic_text_preview();

      #pragma region Handlers: Linkedtopics
         void _add_linked_topic();
         void _move_linked_topic(int by);
         void _remove_linked_topic();
      #pragma endregion

      #pragma region Handlers: Responses
         void _create_response();
         void _edit_response(const QModelIndex& = {});
         void _move_response(int by);
         void _delete_response();
      #pragma endregion

      // Delete key on listviews
      virtual bool eventFilter(QObject* object, QEvent* event) override;
};
