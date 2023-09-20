#pragma once

namespace dovahkit::subsystems::worldinput::config {
   extern double press_to_long_press_threshold(); // seconds

   extern double press_to_hold_threshold(); // seconds

   // Given a sequential keybind like  {A + B}  (i.e. press and release A; then press and 
   // release B), the latter keypress cannot occur more than this many seconds later than 
   // the former keypress,  or else the keybind will not activate.  This rule generalizes 
   // to keybinds consisting of more than two sequential keys;  it defines the max amount 
   // of time that can pass between any two items in the sequence.
   extern double key_sequence_expire_time();
}