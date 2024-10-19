#pragma once
#include <QColor>

namespace DKQuestSceneEditor_impl {
   struct Style {
      bool show_all_text = true;

      int    view_padding = 30;
      int    actor_outset = 20;
      struct {
         QColor color     = { 170, 170, 170 };
         int    thickness = 3;
      } shadow;
      struct {
         QColor background = { 205, 205, 205 };
         QColor border     = { 0, 0, 0 };
         QColor text       = { 0, 0, 0 };
         struct {
            int margin = 5;
         } header_box;
      } phase;
      struct {
         QColor background = { 255, 255, 255 };
         QColor border     = { 0, 0, 0 };
         QColor text       = { 0, 0, 0 };
         int    padding = 10;
      } actor;
      struct { // action
         QColor border = { 0, 0, 0 };
         struct {
            QColor background = { 255, 255, 255 };
            QColor text       = { 0, 0, 0 };
            struct {
               QColor outer_dark  = { 160, 160, 160 };
               QColor outer_light = { 255, 255, 255 };
               QColor inner_dark  = { 105, 105, 105 };
               QColor inner_light = { 227, 227, 227 };
            } border;
            int padding = 3;
         } cell;
         int header_padding = 3;
         int inset = 10;

         struct {
            QColor background = { 205, 205, 205 };
            QColor text       = { 0, 0, 0 };
         } dialogue_header;
         struct {
            QColor background = { 172, 208, 213 };
            QColor text       = { 0, 0, 0 };
         } package_header;
         struct {
            QColor background = { 255, 255, 0 };
            QColor text       = { 0, 0, 0 };
         } timer;
      } action;
   };
}
