#pragma once

// This file exists so that we don't need absolutely every single API file to 
// include coordinator.h, which would make editing the script core very painful. 
// Hopefully, having things go through this file will prevent us from having to 
// recompile absolutely everything whenever we change the core script engine.

namespace dovahscript::tasks {
   class _base;
   class _ui_read_base;
   class _ui_write_base;
}

namespace dovahscript {
   extern void send_script_task(tasks::_base&);
   extern void send_script_ui_task(tasks::_ui_read_base&);
   extern void send_script_ui_task(tasks::_ui_write_base&);
}
