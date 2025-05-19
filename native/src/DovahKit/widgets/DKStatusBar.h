#pragma once
#include <vector>
#include <QColor>
#include <QPointer>
#include <QStatusBar>
#include <QTimer>

class DKStatusBar : public QStatusBar {
   Q_OBJECT;
   Q_PROPERTY(QColor flashColor READ flashColor WRITE setFlashColor DESIGNABLE true);
   Q_PROPERTY(size_t flashCount READ flashCount WRITE setFlashCount DESIGNABLE true);
   #if !defined(QT_PLUGIN)
      Q_PROPERTY(float flashDuration READ flashDuration WRITE setFlashDuration DESIGNABLE true);
      Q_PROPERTY(float flashInterval READ flashInterval WRITE setFlashInterval DESIGNABLE true);
   #else
      //
      // Imagine not supporting floats as a data type in your property-editing UI. 
      // That'd be pretty dumb, right?
      //
      Q_PROPERTY(double flashDuration READ flashDuration WRITE setFlashDuration DESIGNABLE true);
      Q_PROPERTY(double flashInterval READ flashInterval WRITE setFlashInterval DESIGNABLE true);
   #endif
   public:
      DKStatusBar(QWidget* parent = nullptr);

      // Flash the background of a widget contained in the status bar.
      void flash(QWidget*);
      
      #pragma region Properties
         constexpr QColor flashColor() const { return this->_flash.color; }
         void setFlashColor(QColor);

         constexpr size_t flashCount() const { return this->_flash.count; }
         constexpr float flashDuration() const { return this->_flash.duration; }
         constexpr float flashInterval() const { return this->_flash.interval; }

         void setFlashCount(size_t);
         void setFlashDuration(float);
         void setFlashInterval(float);
      #pragma endregion

   protected:
      struct FlashState {
         QPointer<QWidget> target;
         QTimer timer; // single-shot timer for the entire flash animation
      };
      
      struct {
         QColor color;
         QTimer updater; // repeating timer to force visual updates

         size_t count    = 3;   // number of flashes per animation
         float  duration = 0.2; // duration of each flash in seconds
         float  interval = 0.2; // seconds between each flash
      } _flash;
      struct {
         std::vector<FlashState*> flashes;
      } _state;

      bool _contains_widget(const QWidget*) const;
      void _on_flash_update();
      void _remove_dead_flashes();

      virtual void paintEvent(QPaintEvent* event) override;
};