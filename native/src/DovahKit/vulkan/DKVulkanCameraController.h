#pragma once
#include <array>
#include <chrono>
#include <QObject>
#include <QPointer>
#include <QWidget>
#include "data/DKVulkanCameraUpdate.h"

// use as an event filter
class DKVulkanCameraController : public QObject {
   Q_OBJECT;
   public:
      DKVulkanCameraController(QObject* parent = nullptr);

   public slots:
      DKVulkanCameraUpdate poll();

   protected:
      using timestamp_t = std::chrono::time_point<std::chrono::steady_clock, std::chrono::milliseconds>;
      static constexpr timestamp_t not_down = timestamp_t(timestamp_t::duration::zero());

      static constexpr int vk_not_bound = -1;

      struct _Key {
         _Key() {}
         _Key(char c) : vk(c) {}

         int  vk     = vk_not_bound;
         bool ignore = false;
         timestamp_t down_at = not_down;

         inline bool is_down() const noexcept { return !this->ignore && this->down_at != not_down; }
         bool check(timestamp_t); // re-check input state; returns whether the key is down
         void ignore_if_down();
      };

      struct {
         struct {
            struct {
               struct {
                  _Key forward = { 'W' };
                  _Key back    = { 'S' };
                  _Key left    = { 'A' };
                  _Key right   = { 'D' };
                  _Key up      = { 'Q' };
                  _Key down    = { 'Z' };
               } move;
               struct {
                  _Key left  = { 'G' };
                  _Key right = { 'H' };
                  _Key up    = { 'R' };
                  _Key down  = { 'V' };
               } turn;
            } camera;
         } keyboard;
         timestamp_t last_poll = not_down;
      } state;
};