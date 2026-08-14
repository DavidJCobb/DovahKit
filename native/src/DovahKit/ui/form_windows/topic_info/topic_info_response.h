#pragma once
#include <cstdint>
#include <QDialog>
#include "dovah/data/dialogue/emotion.h"
#include "dovah/forms/TopicInfo.h"
#include "ui_topic_info_response.h" // generated
class TopicInfoResponseVoicesModel;

class FormSubdialogTopicInfoResponse : public QDialog {
   Q_OBJECT;
   public:
      using loaded_form_type = dovah::loaded_forms::TopicInfo;
      using response_type    = loaded_form_type::response;

   public:
      FormSubdialogTopicInfoResponse(QWidget* parent = nullptr);

      void importFrom(const loaded_form_type&, const response_type&);
      void exportTo(loaded_form_type&, response_type&);

   protected:
      Ui::FormSubdialogTopicInfoResponse ui;
      struct {
         TopicInfoResponseVoicesModel* voices = nullptr;
      } _models;

      void _on_voicetype_selection_changed();
};