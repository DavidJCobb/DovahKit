#pragma once
#include <QColor>

struct lua_State;

namespace editor_script::util::ui {
   extern void push_color(lua_State* L, const QColor&);
   extern [[nodiscard]] QColor pull_color(lua_State* L, int index);
}