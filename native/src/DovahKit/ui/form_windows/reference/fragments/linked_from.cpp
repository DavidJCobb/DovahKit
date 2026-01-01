#include "./linked_from.h"
#include <QTableView>
#include "widgets/DKHeaderView.h"
#include "dovah/forms/ObjectReference.h"
#include "ui/utils/set_tableview_column_flex.h"
#include "ui/utils/typical_tableview_config.h"
#include "../ObjectReferenceLinkedFromModel.h"

namespace ui::reference::fragments {
   void linked_from::setup(QWidget& owner, const control_collection& controls) {
      this->controls = controls;
      this->model    = new model_type(controls.view);
      
      controls.view->setModel(this->model);
      ui::typical_tableview_config(controls.view);
      ui::set_tableview_column_flex(controls.view, [this](DKHeaderView& header, const QFontMetrics& metrics) {
         for (int col : std::array{
            model_type::Column::KeywordName,
            model_type::Column::RefName,
         }) {
            auto title = this->model->headerData(col, header.orientation(), Qt::DisplayRole).toString();
            header.setColumnFlex(col, 1, 1, metrics.horizontalAdvance(title) * 1.5F + 4);
         }
         header.setColumnFlex(model_type::Column::RefFormID, 0, 0, 4);
      });
   }
   void linked_from::load(loaded_form_type& form) {
      this->stub = &form.stub;

      this->model->setSubject(&form.stub);
   }
   void linked_from::save(loaded_form_type& form) {
   }
}