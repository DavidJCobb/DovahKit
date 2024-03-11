#include "./DKBoundScriptDialog.h"
#include <array>
#include <cassert>
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardItemModel>
#include "helpers/type_traits/is_std_vector.h"
#include "../widget-models/DKBoundScriptModel.h"
#include "../widget-models/DKBoundScriptListModel.h"

#include "editor/form_stub_meta_type.h"
#include "ui/types/quest_alias.h"

namespace {
   constexpr const bool allow_incomplete_polishing = true;
}

namespace vmad {
   using namespace dovah::loaded_forms::components::papyrus;
}

DKBoundScriptDialog::DKBoundScriptDialog(QWidget& parent, QModelIndex scriptModelIndex) : QDialog(&parent) {
   this->ui.setupUi(this);

   this->setWindowFlag(Qt::WindowContextHelpButtonHint);

   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, this, [this]() {
      if (this->script_model) {
         delete this->script_model;
         this->script_model = nullptr;
      }
      this->reject();
   });
   QObject::connect(this->ui.buttonOK, &QPushButton::clicked, this, [this]() {
      ((DKBoundScriptListModel*)this->script_qmi.model())->replaceScriptModelFor(this->script_qmi, this->script_model);
      this->accept();
   });

   static_assert(allow_incomplete_polishing, "POLISH: Show scriptname and form it's attached to in the window title.");

   this->script_qmi   = scriptModelIndex;
   this->script_model = ((DKBoundScriptListModel*)scriptModelIndex.model())->createScriptModelFor(scriptModelIndex);

   this->ui.valueWidget_float->setMinimum(std::numeric_limits<float>::lowest());
   this->ui.valueWidget_float->setMaximum(std::numeric_limits<float>::max());

   {
      auto* prop_view = this->ui.properties;
      prop_view->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
      prop_view->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
      if (scriptModelIndex.isValid()) {
         prop_view->setModel(this->script_model);
      }
      if (auto* header = prop_view->horizontalHeader()) {
         header->setStretchLastSection(true);
      }
      if (auto* vh = prop_view->verticalHeader()) {
         vh->setSectionResizeMode(QHeaderView::ResizeToContents); // needed for sane row sizing
         vh->setVisible(false);
      }
      QObject::connect(prop_view->selectionModel(), &QItemSelectionModel::selectionChanged, [this](const QItemSelection& selected, const QItemSelection& deselected) {
         this->_showSelectedProperty();
      });
      prop_view->setIconSize({ 16, 16 });
   }

   {
      auto* widget = this->ui.arrayTable;
      widget->setCornerButtonEnabled(false);
      widget->setHorizontalHeaderLabels({ tr("#", "array-value table header: index"), tr("Value", "array-value table header: value")});
      if (auto* header = widget->horizontalHeader()) {
         header->setStretchLastSection(true);
      }
      if (auto* vh = widget->verticalHeader()) {
         vh->setSectionResizeMode(QHeaderView::ResizeToContents); // needed for sane row sizing
         vh->setVisible(false);
      }
      QObject::connect(widget, &QTableWidget::currentCellChanged, this, [this](int currentRow, int currentColumn, int previousRow, int previousColumn) {
         auto qmi = this->_selectedPropertyQMI();
         if (!this->script_model || !qmi.isValid()) {
            return;
         }

         const auto info_opt = this->script_model->infoForProperty(qmi);
         if (!info_opt.has_value())
            return;
         const auto& info = info_opt.value();
         if (!info.is_array)
            return;

         const auto value_opt = this->script_model->getPropertyValueElement(qmi, currentRow);
         if (!value_opt.has_value())
            return;
         this->_populate_edit_widgets(value_opt.value());
      });
   }

   QObject::connect(this->ui.buttonAutoFillAll, &QPushButton::clicked, this, [this]() {
      if (!this->script_model)
         return;
      this->script_model->autoFillAllProperties();
      this->_showSelectedProperty();
   });
   QObject::connect(this->ui.buttonValueAutoFill, &QPushButton::clicked, this, [this]() {
      auto qmi = this->_selectedPropertyQMI();
      if (!this->script_model || !qmi.isValid())
         return;
      this->script_model->autoFillProperty(qmi);
      this->_showSelectedProperty();
   });
   QObject::connect(this->ui.buttonValueClearOrMakeLocal, &QPushButton::clicked, this, [this]() {
      auto qmi = this->_selectedPropertyQMI();
      if (!this->script_model || !qmi.isValid())
         return;

      const auto info_opt = this->script_model->infoForProperty(qmi);
      if (!info_opt.has_value())
         return;
      const auto& info = info_opt.value();

      if (info.is_defined()) {
         this->script_model->clearProperty(qmi);
      } else {
         this->script_model->makePropertyLocal(qmi);
      }
      this->_showSelectedProperty();
   });
   QObject::connect(this->ui.buttonValueRevert, &QPushButton::clicked, this, [this]() {
      auto qmi = this->_selectedPropertyQMI();
      if (!this->script_model || !qmi.isValid())
         return;
      this->script_model->revertProperty(qmi);
      this->_showSelectedProperty();
   });

   QObject::connect(this->ui.arrayButtonAdd, &QPushButton::clicked, this, [this]() {
      auto qmi = this->_selectedPropertyQMI();
      if (!this->script_model || !qmi.isValid())
         return;

      auto value_opt = this->script_model->getPropertyValue(qmi);
      if (!value_opt.has_value())
         return;
      auto& value   = value_opt.value();
      bool  changed = false;
      std::visit(
         [&changed](auto& v) {
            using value_type = std::decay_t<decltype(v)>;
            if constexpr (cobb::is_std_vector<value_type>) {
               v.emplace_back();
               changed = true;
            }
         },
         value
      );
      if (changed)
         this->script_model->setPropertyLocalValue(qmi, value);
   });
   QObject::connect(this->ui.arrayButtonRemove, &QPushButton::clicked, this, [this]() {
      auto qmi = this->_selectedPropertyQMI();
      if (!this->script_model || !qmi.isValid())
         return;

      size_t row;
      if (auto opt = this->_currentArrayElementIndex(); opt.has_value()) {
         row = opt.value();
      } else {
         return;
      }

      auto value_opt = this->script_model->getPropertyValue(qmi);
      if (!value_opt.has_value())
         return;
      auto& value   = value_opt.value();
      bool  changed = false;
      std::visit(
         [row, &changed](auto& v) {
            using value_type = std::decay_t<decltype(v)>;
            if constexpr (cobb::is_std_vector<value_type>) {
               if (row >= v.size())
                  return;
               v.erase(v.begin() + row);
               changed = true;
            }
         },
         value
      );
      if (changed)
         this->script_model->setPropertyLocalValue(qmi, value);
   });
   QObject::connect(this->ui.arrayButtonMoveUp, &QPushButton::clicked, this, [this]() {
      this->_move_currently_focused_array_element(false);
   });
   QObject::connect(this->ui.arrayButtonMoveDown, &QPushButton::clicked, this, [this]() {
      this->_move_currently_focused_array_element(true);
   });

   #pragma region Value-change handlers
   QObject::connect(this->ui.valueWidget_bool, &QCheckBox::stateChanged, this, [this](int state) {
      if (this->_selected_property_element_type() != vmad::property_type::boolean)
         return;
      bool checked = (state == Qt::CheckState::Checked);
      this->_set_currently_focused_value(checked);
   });
   QObject::connect(this->ui.valueWidget_float, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
      if (this->_selected_property_element_type() != vmad::property_type::float32)
         return;
      this->_set_currently_focused_value((float)value);
   });
   QObject::connect(this->ui.valueWidget_int, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
      if (this->_selected_property_element_type() != vmad::property_type::integer)
         return;
      this->_set_currently_focused_value(value);
   });
   QObject::connect(this->ui.valueWidget_string, &QPlainTextEdit::textChanged, this, [this]() {
      if (this->_selected_property_element_type() != vmad::property_type::string)
         return;
      this->_set_currently_focused_value(this->ui.valueWidget_string->toPlainText());
   });
   //
   QObject::connect(this->ui.valueWidget_form, &DKFormPicker::formChanged, this, [this](dovah::form_stub* value) {
      const auto info_opt = this->_selected_property_info();
      if (!info_opt.has_value())
         return;
      const auto& info = info_opt.value();
      if (!info.is_form())
         return;
      if (info.is_ref()) // we have a separate widget for selecting refs
         return;
      this->_set_currently_focused_value(value);
   });
   QObject::connect(this->ui.valueWidget_ref, &DKObjectReferencePicker::refChanged, this, [this](dovah::form_stub* ref) {
      const auto info_opt = this->_selected_property_info();
      if (!info_opt.has_value())
         return;
      if (!info_opt.value().is_ref())
         return;
      this->_set_currently_focused_value(ref);
   });
   QObject::connect(this->ui.valueWidget_alias, &DKQuestAliasPicker::aliasChanged, this, [this](const ui::types::quest_alias& alias) {
      const auto info_opt = this->_selected_property_info();
      if (!info_opt.has_value())
         return;
      if (!info_opt.value().is_alias())
         return;
      this->_set_currently_focused_value(alias);
   });
   #pragma endregion

   this->_showSelectedProperty();

   if (this->script_model->anyPropertiesDiscardedOnLoad()) {
      QMessageBox::warning(
         this,
         tr("Some properties have been removed"),
         tr("Some Papyrus property values were discarded, either because the properties in question don't actually exist on the script (as defined in the relevant PEX files) or because the values were invalid.")
      );
      this->script_model->forgetAnyPropertiesWereDiscardedOnLoad(); // so we're only warned once, if we commit our changes
   }
}

bool DKBoundScriptDialog::loadFailed() const {
   if (!this->script_model)
      return false;
   return this->script_model->failedToLoad();
}

//
// Functions for querying info about the selection:
//

std::optional<DKBoundScriptModel::PropertyInfo> DKBoundScriptDialog::_property_info(const QModelIndex& qmi) const {
   if (!this->script_model)
      return {};
   return this->script_model->infoForProperty(qmi);
}
std::optional<DKBoundScriptModel::PropertyInfo> DKBoundScriptDialog::_selected_property_info() const {
   return this->_property_info(this->_selectedPropertyQMI());
}
vmad::property_type DKBoundScriptDialog::_selected_property_element_type() const {
   const auto info_opt = this->_selected_property_info();
   if (!info_opt.has_value())
      return vmad::property_type::none;
   return vmad::scalar_property_type_for(info_opt.value().raw_type);
}

std::optional<size_t> DKBoundScriptDialog::_currentArrayElementIndex() const {
   auto* sm = this->ui.arrayTable->selectionModel();
   if (!sm)
      return {};
   auto rows = sm->selectedRows();
   if (rows.empty())
      return {};
   auto row = rows[0].row();
   if (row < 0)
      return {};
   return row;
}
QModelIndex DKBoundScriptDialog::_selectedPropertyQMI() const {
   if (!this->script_qmi.isValid())
      return {};
   auto* sm = this->ui.properties->selectionModel();
   if (!sm)
      return {};
   auto rows = sm->selectedRows();
   if (rows.empty())
      return {};
   return rows[0];
}

//
// Functions for editing the current value:
//

void DKBoundScriptDialog::_move_currently_focused_array_element(bool move_down) {
   auto qmi = this->_selectedPropertyQMI();
   if (!this->script_model || !qmi.isValid())
      return;

   size_t from;
   if (auto opt = this->_currentArrayElementIndex(); opt.has_value()) {
      from = opt.value();
   } else {
      return;
   }

   if (!move_down && from == 0) // can't move
      return;
   size_t to = move_down ? from + 1 : from - 1;

   auto value_opt = this->script_model->getPropertyValue(qmi);
   if (!value_opt.has_value())
      return;
   auto& value   = value_opt.value();
   bool  changed = false;
   std::visit(
      [from, to, &changed](auto& v) {
         using value_type = std::decay_t<decltype(v)>;
         if constexpr (cobb::is_std_vector<value_type>) {
            size_t size = v.size();
            if (from >= size || to >= size)
               return;
            if constexpr (std::is_same_v<value_type, std::vector<bool>>) { // fucking vector-of-bool...
               bool a = v[from];
               bool b = v[to];
               v[from] = b;
               v[to]   = a;
            } else {
               std::swap(v[from], v[to]);
            }
            changed = true;
         }
      },
      value
   );
   if (changed) {
      const auto blocker = QSignalBlocker(this->ui.arrayTable);

      this->script_model->setPropertyLocalValue(qmi, value);
      this->ui.arrayTable->setCurrentCell(to, 0);
   }
}
void DKBoundScriptDialog::_set_currently_focused_value(const ui::bound_script_models::property_value& v) {
   auto qmi = this->_selectedPropertyQMI();
   if (!this->script_model || !qmi.isValid()) {
      return;
   }

   const auto info_opt = this->script_model->infoForProperty(qmi);
   if (!info_opt.has_value())
      return;
   const auto& info = info_opt.value();
   const auto  row  = this->ui.arrayTable->currentRow();

   if (!info.is_array) {
      this->script_model->setPropertyLocalValue(qmi, v);
   } else {
      this->script_model->setPropertyLocalValueElement(qmi, v, row);
   }
}

//
// Functions for updating the UI state:
//

void DKBoundScriptDialog::_showSelectedProperty() {
   const auto qmi      = this->_selectedPropertyQMI();
   const auto info_opt = this->_property_info(qmi);
   this->_update_autofill_button(info_opt);
   this->_update_clear_edit_button(info_opt);
   this->_update_revert_button(info_opt);
   this->_show_edit_widgets(info_opt);
   if (!info_opt.has_value()) {
      this->_clear_displayed_typename();
      this->ui.valueButtonsLayout->setEnabled(false);
      return;
   }
   this->ui.valueButtonsLayout->setEnabled(true);

   assert(this->script_model != nullptr);

   const auto& info = info_opt.value();
   {
      auto type_name = this->script_model->data(qmi.siblingAtColumn(DKBoundScriptModel::Column::Type), Qt::DisplayRole).toString();
      this->_update_displayed_typename(type_name);
   }

   this->_update_edit_widget_constraints(info);

   if (info.local_status != DKBoundScriptModel::PropertyInfo::LocalStatus::DefinedLocally) {
      return;
   }

   auto value_opt = this->script_model->getPropertyValue(qmi);
   if (!value_opt.has_value())
      return;
   this->_populate_array_table(value_opt.value());
   if (info.is_array) {
      std::visit(
         [this](const auto& v) {
            using value_type = std::decay_t<decltype(v)>;
            if constexpr (cobb::is_std_vector<value_type>) {
               if (v.empty()) {
                  this->ui.singleValueWrap->setEnabled(false);
               } else {
                  this->ui.singleValueWrap->setEnabled(true);

                  size_t i = 0;
                  {
                     auto opt = this->_currentArrayElementIndex();
                     if (opt.has_value()) {
                        i = opt.value();
                        if (i >= v.size())
                           i = 0;
                     }
                  }
                  this->_populate_edit_widgets(v[i]);
               }
            }
         },
         value_opt.value()
      );
   } else {
      this->ui.singleValueWrap->setEnabled(true);
      this->_populate_edit_widgets(value_opt.value());
   }
}

void DKBoundScriptDialog::_clear_displayed_typename() {
   this->ui.propertyType->setText(tr("Property type: <nothing selected>"));
}
void DKBoundScriptDialog::_update_displayed_typename(QString type_name) {
   if (type_name.isEmpty()) {
      this->ui.propertyType->setText(tr("Property type: <unknown>"));
   } else {
      this->ui.propertyType->setText(tr("Property type: %1").arg(type_name));
   }
}

void DKBoundScriptDialog::_update_autofill_button(const std::optional<DKBoundScriptModel::PropertyInfo>& info_opt) {
   auto* widget = this->ui.buttonValueAutoFill;
   if (!info_opt.has_value()) {
      widget->setEnabled(false);
      return;
   }
   auto& info = info_opt.value();
   widget->setEnabled(info.is_form());
}
void DKBoundScriptDialog::_update_clear_edit_button(const std::optional<DKBoundScriptModel::PropertyInfo>& info_opt) {
   auto* widget = this->ui.buttonValueClearOrMakeLocal;
   if (!info_opt.has_value()) {
      widget->setEnabled(false);
      widget->setText(tr("Clear Value"));
      return;
   }
   widget->setEnabled(true);

   auto& info = info_opt.value();
   if (info.is_defined()) {
      widget->setText(tr("Clear Value"));
   } else {
      widget->setText(tr("Edit Value"));
   }
}
void DKBoundScriptDialog::_update_revert_button(const std::optional<DKBoundScriptModel::PropertyInfo>& info_opt) {
   auto* widget = this->ui.buttonValueRevert;
   if (!info_opt.has_value()) {
      widget->setEnabled(false);
      return;
   }
   auto& info = info_opt.value();
   if (info.inherited) {
      widget->setEnabled(info.local_status == DKBoundScriptModel::PropertyInfo::LocalStatus::DefinedLocally);
   } else {
      widget->setEnabled(false);
   }
}

void DKBoundScriptDialog::_populate_array_table(const ui::bound_script_models::property_value& value) {
   this->ui.arrayTable->clearContents();
   bool is_array = false;

   std::visit(
      [this, &is_array](const auto& v) {
         using value_type = std::decay_t<decltype(v)>;

         if constexpr (cobb::is_std_vector<value_type>) {
            using element_type = typename value_type::value_type;

            is_array = true;

            auto* table = this->ui.arrayTable;
            table->setRowCount(v.size());

            for (size_t i = 0; i < v.size(); ++i) {
               {  // Show index
                  auto* item = new QTableWidgetItem;
                  table->setItem(i, 0, item);
                  item->setText(QString::number(i));
               }
               auto* item = new QTableWidgetItem;
               table->setItem(i, 1, item);

               QString text = ui::bound_script_models::stringify(v[i]);
               item->setText(text);
               item->setToolTip(text);
            }
         }
      },
      value
   );
   this->ui.arrayEditingLayout->setVisible(is_array);
   this->ui.arrayEditingLayout->setEnabled(is_array);
}

void DKBoundScriptDialog::_update_edit_widget_constraints(const DKBoundScriptModel::PropertyInfo& info) {
   const auto blockers = std::array{
      QSignalBlocker(this->ui.valueWidget_alias),
      QSignalBlocker(this->ui.valueWidget_form),
      QSignalBlocker(this->ui.valueWidget_ref),
   };

   this->ui.valueWidget_alias->setRequiredScriptname(info.scriptname);
   this->ui.valueWidget_form->setRequiredScriptname(info.scriptname);
   this->ui.valueWidget_ref->setRequiredScriptname(info.scriptname);

   if (info.native_type.has_value())
      this->ui.valueWidget_form->setAllowedFormType(info.native_type.value());
   else
      this->ui.valueWidget_form->allowAllFormTypes();
}

void DKBoundScriptDialog::_populate_edit_widgets(const ui::bound_script_models::property_value& value) {
   const auto blockers = std::array{
      QSignalBlocker(this->ui.valueWidget_bool),
      QSignalBlocker(this->ui.valueWidget_float),
      QSignalBlocker(this->ui.valueWidget_int),
      QSignalBlocker(this->ui.valueWidget_string),

      QSignalBlocker(this->ui.valueWidget_alias),
      QSignalBlocker(this->ui.valueWidget_form),
      QSignalBlocker(this->ui.valueWidget_ref),
   };

   switch (ui::bound_script_models::property_type_for(value)) {
      using enum vmad::property_type;
      case boolean:
         this->ui.valueWidget_bool->setChecked(std::get<bool>(value));
         break;
      case float32:
         this->ui.valueWidget_float->setValue(std::get<float>(value));
         break;
      case integer:
         this->ui.valueWidget_int->setValue(std::get<int32_t>(value));
         break;
      case string:
         this->ui.valueWidget_string->setPlainText(std::get<QString>(value));
         break;
      case object:
         if (std::holds_alternative<ui::types::quest_alias>(value)) {
            auto& item = std::get<ui::types::quest_alias>(value);
            this->ui.valueWidget_alias->setQuestAlias({
               .quest    = item.quest,
               .alias_id = item.alias_id,
            });
         } else {
            dovah::form_stub* stub = std::get<dovah::form_stub*>(value);
            this->ui.valueWidget_form->setFormStub(stub);
            this->ui.valueWidget_ref->setRef(stub);
         }
         break;
   }
}

void DKBoundScriptDialog::_show_edit_widgets(const std::optional<DKBoundScriptModel::PropertyInfo>& info_opt) {
   auto* array_layout = this->ui.arrayEditingLayout;
   auto* stack        = this->ui.singleValueWrap;
   if (!info_opt.has_value()) {
      array_layout->setEnabled(false);
      array_layout->setVisible(false);
      stack->setCurrentWidget(this->ui.valuePage_defaulted);
      return;
   }
   auto& info = info_opt.value();
   if (info.local_status != DKBoundScriptModel::PropertyInfo::LocalStatus::DefinedLocally) {
      array_layout->setEnabled(false);
      array_layout->setVisible(false);
      stack->setCurrentWidget(this->ui.valuePage_defaulted);
      return;
   }
   array_layout->setEnabled(info.is_array);
   array_layout->setVisible(info.is_array);

   const bool is_form = info.is_form();
   //
   this->ui.valuePage_object->setEnabled(is_form);
   this->ui.valuePage_ref->setEnabled(is_form);
   //
   switch (vmad::scalar_property_type_for(info.raw_type)) {
      case vmad::property_type::boolean:
         stack->setCurrentWidget(this->ui.valuePage_bool);
         break;
      case vmad::property_type::float32:
         stack->setCurrentWidget(this->ui.valuePage_float);
         break;
      case vmad::property_type::integer:
         stack->setCurrentWidget(this->ui.valuePage_int);
         break;
      case vmad::property_type::string:
         stack->setCurrentWidget(this->ui.valuePage_string);
         break;
      case vmad::property_type::object:
         if (info.is_alias()) {
            stack->setCurrentWidget(this->ui.valuePage_alias);
         } else if (is_form) {
            if (info.is_ref()) {
               stack->setCurrentWidget(this->ui.valuePage_ref);
            } else {
               stack->setCurrentWidget(this->ui.valuePage_object);
            }
         } else {
            stack->setCurrentWidget(this->ui.valuePage_defaulted);
         }
         break;
      default:
         stack->setCurrentWidget(this->ui.valuePage_defaulted);
         break;
   }
}