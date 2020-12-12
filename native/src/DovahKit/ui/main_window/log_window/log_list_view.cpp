#include "log_list_view.h"
#include <QHeaderView>
#include <QLineEdit>
#include "../../../helpers/qt/strings.h"
#include "../../../dovah/notice_code_list.h"
#include "../../../editor/core.h"
#include "../../../editor/open_window_for_form.h"
#include "../../../editor/helpers/warning_or_error_to_string.h"

namespace {
   QString _read_error_form_id_to_string(const dovah::detailed_notice::relevant_form& form) {
      QString signature = cobb::qt::four_cc_to_string(dovah::form_type_info::lookup(form.type).signature);
      QString local     = QObject::tr("????????", "log window - missing form ID");
      QString fixed     = QObject::tr("--------", "log window - missing form ID");
      if (form.fixedID)
         fixed = QString("%1").arg(form.fixedID, 8, 16, QChar('0')).toUpper();
      if (form.localID) {
         local = QString("%1").arg(form.localID, 8, 16, QChar('0')).toUpper();
         return QObject::tr("[%1][Local:%2][Loaded:%3]").arg(signature).arg(local).arg(fixed);
      }
      return QObject::tr("[%1:%2]").arg(signature).arg(fixed);
   }
}

#pragma region LogListModelItem
LogListModelItem::LogListModelItem(const QString& t) {
   this->type = type_t::text;
   this->text = t;
}
LogListModelItem::LogListModelItem(const dovah::detailed_notice& warning) {
   using notice_code = dovah::notice_code;
   //
   this->data = warning;
   this->type = type_t::detailed_notice;
   if (warning.flags & dovah::detailed_notice::flag::has_cause_file) {
      this->file = QString::fromStdString(warning.cause_file);
   }
   //
   bool non_continuable_success = false;
   text = editor_helpers::warning_or_error_to_string(warning);
   switch (warning.code) {
      case notice_code::save_complete_but_reopen_failed:
      case notice_code::game_conversion_form_cleanup_failed:
      case notice_code::post_save_none_stub_cleanup_failed:
         non_continuable_success = true;
         break;
   }
}
bool LogListModelItem::compare(const dovah::detailed_notice& warning) const noexcept {
   if (this->type != type_t::detailed_notice)
      return false;
   return this->data == warning;
}
bool LogListModelItem::empty() const noexcept {
   return this->text.isEmpty();
}
#pragma endregion

#pragma region LogListModel
LogListModel::LogListModel(QObject* parent) : QAbstractTableModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::fileLoadWarningReceived, this, &LogListModel::loadWarningReceived);
   QObject::connect(&editor, &DovahKitCore::dataAcquireComplete,     this, &LogListModel::dataAcquireComplete);
   QObject::connect(&editor, &DovahKitCore::dataSaveImminent,        this, &LogListModel::dataSaveImminent);
   QObject::connect(&editor, &DovahKitCore::dataSaveComplete,        this, &LogListModel::dataSaveComplete);
   QObject::connect(&editor, &DovahKitCore::dataSaveFailed,          this, &LogListModel::saveErrorReceived);
}

void LogListModel::dataAcquireComplete() {
   int none_stubs = 0;
   DovahKitCore::get().for_each_form_of_type(dovah::form_type::none, [&none_stubs](dovah::form_stub* stub) {
      if (stub->is_none_stub())
         ++none_stubs;
      return false;
   });
   if (none_stubs) {
      this->addTextEntry(
         tr("Forms in the loaded files contain dangling references to %1 non-existent form(s). Check the \"Missing\" category in the Object Window for a list of the missing forms' form IDs, and view the Use Info on entries to see what's referring to them. It's normal for official game files to have this problem.", "log window")
            .arg(none_stubs)
      );
   }
   //
   this->addTextEntry(tr("All files have been loaded.", "log window"));
}
void LogListModel::dataSaveImminent() {
   this->addTextEntry(tr("Saving active file...", "log window"));
}
void LogListModel::dataSaveComplete() {
   this->addTextEntry(tr("The active file has been successfully saved.", "log window"));
}
void LogListModel::saveErrorReceived(const dovah::detailed_notice& error) {
   auto* item = new item_type(error);
   if (item->empty()) {
      delete item;
      return;
   }
   //
   auto first_inserted = this->children.size();
   auto last_inserted  = first_inserted;
   this->beginInsertRows(QModelIndex(), first_inserted, last_inserted);
   this->children.push_back(item);
   this->endInsertRows();
}
void LogListModel::loadWarningReceived(const dovah::detailed_notice& warning) {
   using flag = dovah::detailed_notice::flag;
   //
   if (warning.context == dovah::detailed_notice::notice_context::on_demand_form_load) {
      //
      // Don't log warnings from on-demand form loads, unless the specific data is 
      // coalesced.
      //
      if (!warning.is_winning_record())
         return;
   }
   //
   auto& map = this->warnings_cause_by_form;
   auto  cause_form_id  = warning.cause_form.fixedID;
   bool  has_cause_form = warning.flags & flag::has_cause_form;
   if (has_cause_form && cause_form_id) {
      auto it = map.find(cause_form_id);
      if (it != map.end()) {
         auto& list = *it;
         for (auto* item : list) {
            if (item->compare(warning))
               return;
         }
      }
   }
   auto* item = new item_type(warning);
   if (item->empty()) {
      delete item;
      return;
   }
   //
   auto first_inserted = this->children.size();
   auto last_inserted  = first_inserted;
   this->beginInsertRows(QModelIndex(), first_inserted, last_inserted);
   //
   this->children.push_back(item);
   if (has_cause_form && cause_form_id) {
      auto& list = map[cause_form_id];
      list.push_back(item);
   }
   //
   this->endInsertRows();
}
void LogListModel::gameSettingValueChangeFailed(const char* name, dovah::notice_code_t code) {
   auto* item = new item_type();
   switch (code) {
      case dovah::notice_code::form_id_unavailable_for_game_setting:
         item->text = tr("An error occurred while trying to modify the value of game setting %1. DovahKit was unable to allocate a form ID for the setting.").arg(name);
         break;
      case dovah::notice_code::game_setting_edit_request_lacked_id:
         item->text = tr("An error occurred while trying to modify the value of game setting %1. No form ID was allocated for the setting.").arg(name);
         break;
      case dovah::notice_code::form_id_is_already_in_use:
         item->text = tr("An error occurred while trying to modify the value of game setting %1. The desired form ID is already in use by another (non-setting) form.").arg(name);
         break;
      case dovah::notice_code::form_id_is_reserved_for_other_process:
         item->text = tr("An error occurred while trying to modify the value of game setting %1. The desired form ID is reserved for use in some other process, such as form creation or form renumbering.").arg(name);
         break;
      default:
         item->text = tr("An unknown error occurred while trying to modify the value of game setting %1.").arg(name);
         break;
   }
   //
   auto first_inserted = this->children.size();
   auto last_inserted  = first_inserted;
   this->beginInsertRows(QModelIndex(), first_inserted, last_inserted);
   this->children.push_back(item);
   this->endInsertRows();
}

QModelIndex LogListModel::index(int row, int column, const QModelIndex& parent) const {
   if (!this->hasIndex(row, column, parent))
      return QModelIndex();
   item_type* childItem = this->children.value(row);
   if (childItem)
      return this->createIndex(row, column, childItem);
   return QModelIndex();
}
QModelIndex LogListModel::parent(const QModelIndex& index) const {
   return QModelIndex();
}
int LogListModel::rowCount(const QModelIndex& parent) const {
   if (parent.column() > 0)
      return 0;
   return this->children.size();
}
int LogListModel::columnCount(const QModelIndex& item) const {
   return 2;
}
Qt::ItemFlags LogListModel::flags(const QModelIndex& index) const {
   if (!index.isValid())
      return Qt::NoItemFlags;
   return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
}
QVariant LogListModel::data(const QModelIndex& index, int role) const {
   if (!index.isValid())
      return QVariant();
   auto item   = (item_type*)index.internalPointer();
   auto column = index.column();
   switch (column) {
      case 0:
         if (role == Qt::DisplayRole)
            return item->text;
         break;
      case 1:
         if (role == Qt::DisplayRole)
            return item->file;
         break;
   }
   return QVariant();
}
inline const LogListModel::item_type* LogListModel::row(int rowIndex) const noexcept {
   return this->children.value(rowIndex);
}
//
QVariant LogListModel::headerData(int section, Qt::Orientation orientation, int role) const {
   if (orientation != Qt::Orientation::Horizontal)
      return QVariant();
   switch (role) {
      case Qt::DisplayRole:
         switch (section) {
            case 0: return tr("Text", "log window header");
            case 1: return tr("File", "log window header");
         }
         break;
   }
   return QVariant();
}

void LogListModel::addTextEntry(const QString& text) {
   auto* item = new item_type(text);
   //
   auto first_inserted = this->children.size();
   auto last_inserted  = first_inserted;
   this->beginInsertRows(QModelIndex(), first_inserted, last_inserted);
   this->children.push_back(item);
   this->endInsertRows();
}
void LogListModel::clear() {
   this->beginResetModel();
   for (auto* item : this->children)
      delete item;
   this->children.clear();
   this->warnings_cause_by_form.clear();
   this->endResetModel();
}
#pragma endregion

#pragma region LogList
LogList::LogList(QWidget* parent) : QTableView(parent) {
   this->setModel(new model_type(this));
   this->verticalHeader()->setDefaultSectionSize(0);
   //
   // The next call is needed for proper word-wrapping in table cells. The "wordWrap" 
   // property on table cells enables word-wrapping if the cell is tall enough, but 
   // doesn't actually resize table cells, so by default, the table behaves exactly 
   // as if word-wrapping were disabled. The next call automatically resizes cells 
   // by way of the vertical header: even if we disable the vertical header, every 
   // row still has a vertical header section associated with it, and that can be 
   // configured to resize.
   //
   // Naturally, pretty much none of this information is mentioned in the Qt docs 
   // for QTableView::setWordWrap, at least as of this writing.
   //
   this->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents); // needed for proper word-wrapping in table cells
   //
   auto header  = this->horizontalHeader();
   auto metrics = QFontMetrics(this->font());
   header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignBaseline);
   header->setMinimumSectionSize(2);
   header->setSectionResizeMode(0, QHeaderView::Stretch);
   header->setSectionResizeMode(1, QHeaderView::Interactive);
};
#pragma endregion