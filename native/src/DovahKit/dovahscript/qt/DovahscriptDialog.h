#pragma once
#include <atomic>
#include <QDialog>

class DovahscriptDialog : public QDialog {
   Q_OBJECT;
   public:
      using QDialog::QDialog;
   protected:
      std::atomic<bool> _visible;

   public:
      virtual void setVisible(bool) override;

      // thread-safe test
      inline bool lastVisibleState() const noexcept {
         return this->_visible;
      }

   signals:
      void shown();
      void hidden();
};