local window = ui.window.new()
local widget = ui.table_view.new()
window:set_layout("grid")
window:add_child(widget)
widget:append_row("Hello!")
window:show()

local cell = widget.rows[1].cells[1]

function _validate(name)
   local c = cell.text_color
   if c.r == 255 and c.g == 0 and c.b == 192 then
      dovah.log_message(name .. ": PASS")
      return
   end
   dovah.log_message(name .. ": FAIL? RGBA: (%d, %d, %d, %d)", c.r, c.g, c.b, c.a)
   return
end

cell.text_color = "#FF00C0"
_validate("hex 6")
cell.text_color = "#FF00C0FF"
_validate("hex 8")
cell.text_color = { r = 255, g = 0, b = 192 }
_validate("table")
cell.text_color = "rgb(255, 0, 192)"
_validate("function RGB")
cell.text_color = "rgb(255 0 192)"
_validate("function RGB space")
cell.text_color = "rgba(255, 0, 192, 0.5)"
_validate("function RGBA")
cell.text_color = "rgba(255 0 192 / 0.5)"
_validate("function RGBA space")
cell.text_color = "hsl(315, 100%, 50%)"
_validate("function HSL")
cell.text_color = "hsla(315, 100%, 50%, 0.5)"
_validate("function HSLA")