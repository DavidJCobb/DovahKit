subject = ui.line.new("horizontal")

local window = ui.window.new()
local pad_a  = ui.widget.new()
local pad_b  = ui.widget.new()

pad_a.min_height = 32
pad_a.min_width  = 32
--
pad_b.min_height = 32
pad_b.min_width  = 32

window:set_layout("down")
window:add_child(pad_a)
window:add_child(subject)
window:add_child(pad_b)

function _layout(o)
   if o == "horizontal" then
      window:set_layout("down")
      window:add_child(pad_a)
      window:add_child(subject)
      window:add_child(pad_b)
   elseif o == "vertical" then
      window:set_layout("ltr")
      window:add_child(pad_a)
      window:add_child(subject)
      window:add_child(pad_b)
   end
end
_layout(subject.direction)

window:show()

-- call via eval:
function set_direction(o)
   subject.direction = o
   _layout(subject.direction) -- read in order to normalize
end