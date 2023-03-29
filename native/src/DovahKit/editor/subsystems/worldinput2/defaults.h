#pragma once

namespace dovahkit::subsystems::worldinput2::defaults {
   constexpr const double press_to_long_press_threshold = 0.35; // seconds
   constexpr const double long_press_to_hold_threshold  = 0.50; // seconds

   constexpr const double press_to_hold_threshold = press_to_long_press_threshold + long_press_to_hold_threshold;

   // Given a sequential keybind like  {A + B}  (i.e. press and release A; then press and 
   // release B), the latter keypress cannot occur more than this many seconds later than 
   // the former keypress,  or else the keybind will not activate.  This rule generalizes 
   // to keybinds consisting of more than two sequential keys;  it defines the max amount 
   // of time that can pass between any two items in the sequence.
   constexpr const double key_sequence_expire_time = 0.25;
}