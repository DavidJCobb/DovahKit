#include "DKGameFilePicker.h"
#include <QAction>
#include <QDir>
#include <QEvent>
#include <QHBoxLayout>
#include <QStyleOptionButton>
#if !defined(QT_DESIGNER_LIB)
   #include "widget-dialogs/DKBSABrowseDialog.h"
#endif

DKGameFilePicker::DKGameFilePicker(QWidget* parent) : QWidget(parent) {
   auto* layout   = new QHBoxLayout(this);
   auto* w_path   = this->subwidgets.path   = new QLineEdit(this);
   auto* w_browse = this->subwidgets.browse = new QPushButton(tr("...", "browse button label"), this);
   layout->setContentsMargins({ 0, 0, 0, 0 });
   layout->addWidget(w_path, 1);
   layout->addWidget(w_browse, 0);
   #if !defined(QT_DESIGNER_LIB)
      QObject::connect(w_browse, &QPushButton::clicked,  this, &DKGameFilePicker::browse);
      QObject::connect(w_path,   &QLineEdit::textEdited, this, [this](const QString& path) {
         this->_setPath(path, true, false);
      });
   #endif
   this->_updateBrowseButtonSize();
   //
   #if !defined(QT_DESIGNER_LIB)
      {  // Hook the "clear" button
         w_path->setClearButtonEnabled(false);
         assert(w_path->actions().isEmpty());
         w_path->setClearButtonEnabled(true);
         auto actions = w_path->actions();
         if (actions.size() == 1) {
            auto* action = actions[0];
            QObject::connect(action, &QAction::triggered, this, &DKGameFilePicker::clear);
         }
      }
   #endif
   //
   w_browse->setAccessibleName(tr("Browse..."));
   w_browse->setAccessibleDescription(tr("Open a dialog to select a game asset file from the currently loaded BSA archives."));
   //
   this->setFocusPolicy(Qt::FocusPolicy::TabFocus);
   this->setFocusProxy(w_path);
   QWidget::setTabOrder(w_path, w_browse);
}

QString DKGameFilePicker::effectiveStem() const noexcept {
   if (!this->state.stem.isEmpty())
      return this->state.stem;
   switch (this->state.config) {
      using _ = StandardConfiguration;
      case _::NoConfiguration:
         break;
      case _::Meshes:
         return "meshes/";
      case _::Textures:
         return "textures/";
   }
   return QString();
}
QString DKGameFilePicker::path(PathFormat v) const noexcept {
   auto path = this->state.value;
   if (path.isEmpty() || v == PathFormat::WidgetPathFormat)
      return path;
   if (v == PathFormat::ShowDataDirectory) {
      path = "Data/" + path;
   } else if (v == PathFormat::OmitPathStem) {
      const auto stem = this->effectiveStem();
      if (!stem.isEmpty() && path.startsWith(stem)) {
         path = path.mid(stem.size());
         if (!path.isEmpty() && path.startsWith('/'))
            path = path.mid(1);
      }
   }
   return path;
}
QString DKGameFilePicker::rawPath() const noexcept {
   auto path = this->path(this->state.displayFormat);
   path.replace('/', '\\');
   return path;
}

/*static*/ QString DKGameFilePicker::normalizePath(const QString& path) noexcept {
   auto cleaned = QDir::cleanPath(path);
   if (cleaned.isEmpty())
      return cleaned;
   //
   // Clean up the path:
   // 
   //  - QDir::cleanPath to normalize
   //  - Strip leading slashes
   //  - Strip the "data" directory
   //
   int i = 0;
   if (cleaned.startsWith('/')) {
      ++i;
   }
   if (cleaned.size() > i + 4) {
      if (cleaned.midRef(i, 4).compare(QLatin1String("data"), Qt::CaseInsensitive) == 0) {
         i += 4;
         if (cleaned.size() > i + 1)
            if (cleaned[i] == '/')
               ++i;
      }
   }
   if (i)
      cleaned = cleaned.mid(i);
   return cleaned;
}

void DKGameFilePicker::browse() {
   #if !defined(QT_DESIGNER_LIB)
      auto result = DKBSABrowseDialog::getOpenFileName(
         this,
         tr("Select file"),
         this->effectiveStem(),
         this->state.value,
         QString(),
         nullptr,
         {
            .validationOptions = this->validationOptions(),
         }
      );
      result = QDir::cleanPath(result);
      if (result.isEmpty())
         return;
      this->setPath(result);
   #endif
}
void DKGameFilePicker::clear() {
   bool change = !this->state.value.isEmpty();
   this->state.value = QString();
   //
   auto* widget = this->subwidgets.path;
   auto  blocker = QSignalBlocker(widget);
   widget->clear();
   //
   if (change)
      emit this->pathChanged(QString());
}
void DKGameFilePicker::setPath(const QString& path) {
   this->_setPath(path, false, true);
}
void DKGameFilePicker::setPathFormat(PathFormat v) {
   if (this->state.displayFormat == v)
      return;
   this->state.displayFormat = v;
   //
   auto* widget  = this->subwidgets.path;
   auto  blocker = QSignalBlocker(widget);
   widget->setText(this->path(v));
}
void DKGameFilePicker::setPathStem(const QString& stem) {
   this->state.stem = QDir::cleanPath(stem);
}
void DKGameFilePicker::setRawPath(const QString& path) {
   this->_setPath(path, true, true);
}
void DKGameFilePicker::setStandardConfiguration(StandardConfiguration sc) {
   if (this->state.config == sc)
      return;
   this->state.config = sc;
}
void DKGameFilePicker::setValidationOptions(ValidationOptions v) {
   this->state.validationOptions = v;
}

bool DKGameFilePicker::eventFilter(QObject* watched, QEvent* event) {
   if (watched == this->subwidgets.path) {
      auto* widget = this->subwidgets.path;
      if (event->type() == QEvent::Type::FocusOut) {
         widget->setText(this->path(this->state.displayFormat));
         return false;
      }
      return false;
   }
   if (watched == this->subwidgets.browse) {
      switch (event->type()) {
         case QEvent::FontChange:
         case QEvent::MacSizeChange:
         case QEvent::StyleChange:
            this->_updateBrowseButtonSize();
            break;
      }
      return false;
   }
   return false;
}

void DKGameFilePicker::_emitPathChangeSignals() {
   auto rp = this->rawPath();
   emit this->pathChanged(this->state.value);
   emit this->rawPathChanged(rp);
}
void DKGameFilePicker::_setPath(const QString& path, bool isRawPath, bool updateTextbox) {
   auto cleaned = normalizePath(path);
   if (isRawPath) {
      if (this->pathFormat() == PathFormat::OmitPathStem) {
         auto stem = this->effectiveStem();
         if (!stem.isEmpty()) {
            bool needs_slash = stem.back() != '/';
            if (needs_slash)
               if (!cleaned.isEmpty() && cleaned[0] == '/')
                  needs_slash = false;
            if (!needs_slash) {
               cleaned = stem + cleaned;
            } else {
               cleaned = stem + '/' + cleaned;
            }
         }
      }
   }
   if (this->state.value == cleaned)
      return;
   this->state.value = cleaned;
   //
   if (updateTextbox) {
      auto* widget  = this->subwidgets.path;
      auto  blocker = QSignalBlocker(widget);
      widget->setText(this->path(this->state.displayFormat));
   }
   //
   this->_emitPathChangeSignals();
}
void DKGameFilePicker::_updateBrowseButtonSize() {
   auto* widget = this->subwidgets.browse;
   widget->ensurePolished();
   //
   auto* style = widget->style();
   if (!style)
      return;
   auto text  = widget->text();
   auto fm    = QFontMetrics(widget->font());
   auto width = fm.boundingRect(text).width();
   //
   QStyleOptionButton opt;
   opt.initFrom(widget);
   std::swap(opt.text, text);
   auto frame = style->pixelMetric(QStyle::PM_DefaultFrameWidth, &opt, widget);
   frame += style->pixelMetric(QStyle::PM_ButtonMargin, &opt, widget);
   frame *= 2;
   //
   widget->setMaximumSize({ frame + width, QWIDGETSIZE_MAX });
}