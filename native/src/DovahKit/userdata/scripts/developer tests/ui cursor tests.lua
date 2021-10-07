local CURSORS <const> = {
   "normal",
   "crosshair",
   "wait",
   "resize, v",
   "resize, h",
   "pointer",
   "forbidden",
   "hand, open",
   "hand, closed",
   "what's this?",
   "busy in background",
}

window = ui.window.new()
window:set_layout("grid")

local size    = #CURSORS
local per_col = math.ceil(size / 4)
for i = 1, size do
   local y = i % per_col
   local x = math.floor(i / per_col)
   --
   local button = ui.button.new(CURSORS[i])
   window:add_child(button, y + 1, x + 1)
   button.cursor = CURSORS[i]
end
window:show()