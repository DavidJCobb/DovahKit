#pragma once
#include <QDialog>
#include <QUrl>

class ScriptWindowHyperlinkConfirmDialog : public QDialog {
   Q_OBJECT;
   public:
      ScriptWindowHyperlinkConfirmDialog(const QString& url, QWidget* parent = nullptr);
   protected:
      QUrl _targetUrl;
};